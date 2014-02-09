#include "TimeRecorder_pub.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

///! all internal typedef or macro should be move to another private definition file in the future

///!
typedef struct _TIME_CLASS_MAP_EXTEND{
    TIME_CLASS_MAP mTCM;        /*!< client give this information(客户端指定结构) */
    unsigned int timeCounter;   /*!< time loged on this class */
}TCME,*PTR_TCME;

typedef struct _Client{
    ClientID mID;               /*!< client identify */
    PTR_TCME mTCM;              /*!< client custom time classes*/
    unsigned int mMapSize;      /*!< client custom time classes count*/
    unsigned int totalTimeCounter;
    struct _Client* pNext;
    struct _Client* pPre;
}Client,*Ptr_Client;

typedef struct _TimeRecorder{
    Ptr_Client pCurrentClient;
}TimeRecorder;

static TimeRecorder gTimer = {0};   /*!< 全局的Timer链表 */

///! 私有函数定义：主要是数据结构操作
static inline Ptr_Client newClient( const ClientID aClientID );
//aBefore == NULL 表示我想要把节点直接放在pCurrentClient前面（如果不是空链表）
static bool insertClientBefore( Ptr_Client aInsert, Ptr_Client aBefore );
//aBefore == NULL 表示我想要把节点直接放在pCurrentClient后面（如果不是空链表）
//static bool insertClientAfter( Ptr_Client aInsert, Ptr_Client aAfter );
static bool removeClient( Ptr_Client aRemove );
static Ptr_Client findClient( const ClientID );

// startTimeRecorder
bool startTimeRecorder( const ClientID aClientID ){
    bool rValue;
    Ptr_Client clientNode;

    rValue = false;
    clientNode = NULL;

SINGLE_FLOW_BEGIN
    if( findClient(aClientID) == NULL ){
        clientNode = newClient( aClientID );

        if( NULL == clientNode ) break;
        if( !insertClientBefore( clientNode, NULL ) ) break;

        rValue = true;
    }
    else{
        //已经注册过的再次注册，在这种默认注册的情况下，就放行
        rValue = true;
    }
SINGLE_FLOW_END
    return rValue;
}//! end of startTimeRecorder()

/// startTimeRecorderWithClass
bool startTimeRecorderWithClass
    (
     const ClientID aClientID
     , const TIME_CLASS_MAP* aTCMap
     , unsigned int len
     ){
    bool rValue;
    Ptr_Client clientNode;
    TIME_CLASS_MAP* pMap;

    rValue = false;
    clientNode = NULL;
    pMap = NULL;
SINGLE_FLOW_BEGIN
    if( NULL == aTCMap || 0 == len ){
        break;
    }
    if( NULL == findClient( aClientID ) ){
        clientNode = newClient( aClientID );

        if( NULL == clientNode ) break;
        if( !insertClientBefore( clientNode, NULL ) ) break;

        PTR_TCME pNewMap = (PTR_TCME)malloc( sizeof(TCME)*len );
        memset( (void*)pNewMap, 0, sizeof(TCME)*len );
        clientNode->mTCM = pNewMap;
        clientNode->mMapSize = len;

        // 复制client端提供的记录类型信息
        for( pMap = aTCMap; pMap < aTCMap + len; ++pMap ){

            pNewMap->mTCM.szClassName = pMap->szClassName;
            pNewMap->mTCM.tClassID = pMap->tClassID;
            pNewMap->mTCM.pClassName = (char*)malloc( sizeof(char)* pMap->szClassName + 1 );
            memset( pNewMap->mTCM.pClassName, 0, sizeof(char)* pMap->szClassName + 1 );
            strncpy( pNewMap->mTCM.pClassName, pMap->pClassName, pMap->szClassName );
            pNewMap->timeCounter = 0;

            pNewMap++;
        }
        assert( pMap == aTCMap + len );
        //! init time class map
        rValue = true;
    }
    else{
        assert(false);  // client 端不应该连续调用该函数来注册自己
        rValue = false;
    }
SINGLE_FLOW_END

    return rValue;
}

/// stopTimerRecorder()
bool stopTimerRecorder( const ClientID aClientID )
{
    bool rValue;
    Ptr_Client clientNode;

    rValue = false;
    clientNode = NULL;
SINGLE_FLOW_BEGIN
    clientNode = findClient( aClientID );
    if( clientNode != NULL )
    {
        PTR_TCME pTCMap = clientNode->mTCM;
        if( clientNode->mMapSize != 0 )
        {
            for(; pTCMap < clientNode->mTCM + clientNode->mMapSize; ++pTCMap )
            {
                if( pTCMap->mTCM.pClassName ) free( pTCMap->mTCM.pClassName );
            }
            free( clientNode->mTCM );
        }
        removeClient( clientNode );
        rValue = true;
    }
    else
    {
        assert( false );    //不应该去释放一个从未注册的clien
    }
SINGLE_FLOW_END

    return rValue;
}//End stopTimerRecorder()

//logTime()
bool logTime( ClientID aClientID, unsigned int aTime ){
    bool rValue;
    Ptr_Client clientNode;

    rValue = false;
    clientNode = findClient( aClientID );
    if( clientNode ){
        clientNode->totalTimeCounter += aTime;
        rValue = true;
    }
    else{
        assert( false );
    }

    return rValue;
}// end logTime()

//logTimeClass()
bool logTimeClass
    (
     const ClientID aClientID
     , unsigned int aTCID
     , unsigned int aTime
    ){
bool rValue;
Ptr_Client clientNode;
PTR_TCME pMap;

rValue = false;
clientNode = findClient( aClientID );

SINGLE_FLOW_BEGIN

BREAK_IF_FALSE( clientNode != NULL );
BREAK_IF_FALSE( clientNode->mTCM != NULL && clientNode->mMapSize > 0 );

pMap = clientNode->mTCM;
BREAK_IF_FALSE( pMap != NULL );
for( ; pMap < clientNode->mTCM + clientNode->mMapSize; pMap++ ){

    if( pMap->mTCM.tClassID == aTCID ){
        pMap->timeCounter += aTime;
        break;
    }
}
clientNode->totalTimeCounter += aTime;  //记录到总时间

BREAK_IF_FALSE( pMap < clientNode->mTCM + clientNode->mMapSize );   //! can't find given time class
rValue = true;

SINGLE_FLOW_END

return rValue;
}

//buildReport( )
bool buildReport( const ClientID aClientID, char* aRpt ){
bool rValue;
Ptr_Client ptrClientNode;
PTR_TCME ptrMap;

const char* TITLE = "\nTime Record Report Start -->\n";
const char* Total_Format = "Total time record : %d\n";
const char* Each_Class_Format = "[%s] record : %d\n";
const char* Tail = "Time Record Report End <--\n";

aRpt = NULL;
rValue = false;
ptrClientNode = findClient( aClientID );
SINGLE_FLOW_BEGIN
BREAK_IF_TRUE( ptrClientNode == NULL );

ptrMap = ptrClientNode->mTCM;

//0. build Header string
if( NULL == aRpt ){
    printf( "%s",TITLE );
}
//1. build Total information
    {
    //TO-DO 如何获得准确的字串长度？
    char* pTotalInfor = (char*)malloc( sizeof(char)*( strlen( Total_Format ) + 10 ) );
    memset( pTotalInfor, 0, sizeof(char)*( strlen( Total_Format ) + 10 ) );
    sprintf( pTotalInfor, Total_Format, ptrClientNode->totalTimeCounter );
    printf( "%d\n", ptrClientNode->totalTimeCounter );
    if( aRpt == NULL ){
        printf( "%s",pTotalInfor );
        }
    }
//2. build each class if any
if( ptrMap ){
    for( ;ptrMap < ptrClientNode->mTCM + ptrClientNode->mMapSize; ++ptrMap ){
        char* pItem;
        unsigned int szItem;

        szItem = sizeof(char)*                          \
                (                                       \
                 ptrMap->mTCM.szClassName   \
                 + strlen( Each_Class_Format )          \
                 + 10                                   \
                 );
        pItem = (char*)malloc( szItem );
        sprintf( pItem, Each_Class_Format, ptrMap->mTCM.pClassName, ptrMap->timeCounter );
        if( aRpt == NULL ){
            printf( "%s", pItem );
        }
    }// End for
}// end if
else{
    printf("?????????????????????????\n");
}

//3. build tail
if( NULL == aRpt ){
    printf( "%s", Tail );
}

rValue = true;
SINGLE_FLOW_END

return rValue;
}

// Private Function implementation
Ptr_Client newClient( const ClientID aClientID ){
    Ptr_Client pClient = (Ptr_Client)malloc( sizeof( Client ) );

    if( NULL == pClient ){
        assert( false );
        return NULL;
    }

    memset( pClient, 0, sizeof( Client ) );
    pClient->mID = aClientID;

    return pClient;
}

/////////////////////////////////////////////////
/// \brief 插入一个节点
///
/// \param aClient : pointer to client need insert
/// \param before : where to insert
/// \return true if insert successful else false
///
/////////////////////////////////////////////////
bool insertClientBefore( Ptr_Client aClient, Ptr_Client before ){
    bool rValue;
    Ptr_Client p;

    rValue = false;

    SINGLE_FLOW_BEGIN
    BREAK_IF_TRUE( aClient == NULL );
    if( NULL == before ){
        p = gTimer.pCurrentClient;
    }
    else{
        while( p != NULL && p != before ){
            p = p->pNext;
        }
        if( p != before ){
            p = gTimer.pCurrentClient;
            while( p != NULL && p != before ){
                p = p->pPre;
            }
        }
        if( p != before ){//can't find before in list
            assert( false );
            break;
        }
    }

    if( NULL == p ){//Empty Link list
        gTimer.pCurrentClient = aClient;
    }
    else{
        aClient->pPre = p->pPre;
        p->pPre->pNext = aClient;
        p->pPre = aClient;
        aClient->pNext = p;
    }

    rValue = true;

    SINGLE_FLOW_END

    return rValue;
}

/////////////////////////////////////////////////
/// \brief find client with given client id
///
/// \param aClientID client identify
/// \return Ptr_Client NULL if not find
///
/////////////////////////////////////////////////
Ptr_Client findClient( const ClientID aClientID ){
    Ptr_Client rPtr;

    rPtr = gTimer.pCurrentClient;
SINGLE_FLOW_BEGIN
    BREAK_IF_TRUE( rPtr == NULL );

    while( rPtr != NULL && rPtr->mID != aClientID ){
        rPtr = rPtr->pNext;
    }
    if( !rPtr ){
            rPtr = gTimer.pCurrentClient->pPre;
        while( rPtr != NULL && rPtr->mID != aClientID ){
            rPtr = rPtr->pPre;
        }
    }
SINGLE_FLOW_END

    return rPtr;
}

/////////////////////////////////////////////////
/// \brief
///
/// \param
/// \param
/// \return
///
/////////////////////////////////////////////////
bool removeClient( Ptr_Client deleteNode ){
    bool rValue;

    rValue = false;

    SINGLE_FLOW_BEGIN
    if( findClient( deleteNode->mID ) )
    {
        if( deleteNode->pPre ) deleteNode->pPre->pNext = deleteNode->pNext;
        if( deleteNode->pNext ) deleteNode->pNext->pPre = deleteNode->pNext;
        if( gTimer.pCurrentClient == deleteNode ){
            gTimer.pCurrentClient = ( deleteNode->pPre ) ? ( deleteNode->pPre ) : deleteNode->pNext;
        }

        free( deleteNode );
        rValue = true;
        break;
    }
    SINGLE_FLOW_END

    return rValue;
}
