#include "TimeRecorder_pub.h"

#include "Common.h"

///! all internal typedef or macro should be move to another private definition file in the future
typedef struct _TIME_CLASS_MAP_EXTEND{
    TIME_CLASS_MAP mTCM;        /*!< client give this information */
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
static bool insertClientAfter( Ptr_Client aInsert, Ptr_Client aAfter );
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

        //!!!todo error
        for( pMap = aTCMap; pMap < aTCMAP + len; ++pMap ){

            unsigned int len_name = pMap->szClassName;

            pNewMap->szClassName = len_name;
            pNewMap->tClassID = aClientID;
            pNewMap->pClassName = (char*)malloc( sizeof(char)* pMap->szClassName );
            strncpy( pNewMap->pClassName, pMap->pClassName, pMap->szClassName );

            pNewMap++;
        }

        //! init time class map
        rValue = true;
    }
    else{
        rValue = true;
    }


SINGLE_FLOW_END

    return rValue;
}

