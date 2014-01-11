#include "GCache_pub.h"

#include "stdio.h"
#include "stdlib.h"
#include "assert.h"

#if( _DEBUG_GCACHE_ )
#define Debug_Printf(_p) printf(_p)
#else
#define Debug_Printf(_msg) do{}while(0)
#endif

typedef char* Ptr_Byte;
typedef struct _itrnl
{
    void* pCacheMemory;				///!client指定的用作cache用途的内存地址
    unsigned int mSzBytes;			///!memory的大小，字节为单位
    void* pFirstNode;
    void* pLastNode;
    void* pCurInsertPos;            /**< 当前可以插入记录的位置 */
} GCache_Contrl_Block, *Ptr_GCache_Control_Block;

typedef GCache_Contrl_Block GCCB;
typedef Ptr_GCache_Control_Block PGCCB;

//param is _itrnl
#define GET_NODE_START_ADDRESS( _Mem ) (void*)( 							\
										(Ptr_Byte)( ((PGCCB)_Mem)->pCacheMemory ) 	\
										+ sizeof( GCCB )					\
										);

#define GET_NODE_END_ADDRESS( _Mem ) (void*)(									\
											(Ptr_Byte)( ((PGCCB)_Mem)->pCacheMemory )	\
											+ ( (PGCCB)_Mem )->mSzBytes			\
											)
//param is This
#define PTR_GCCB_MEM(_This) ( (PGCCB)(_This)->pIntrl )

#define PTR_START_OF_CACHE_CONTENT(_This) ( ( (Ptr_Byte)( (PGCCB)( (_This)->pIntrl )->pCacheMemory ) ) + sizeof( GCCB ) )

#define PTR_END_OF_CACHE_CONTEXT(_This) ( (char*)PTR_START_OF_CACHE_CONTENT(_This) + (PGCCB)(_This)->mSzBytes - sizeof( GCCB ) )

#define MIN_NODE_CNT 10

#define MATH_ROUND_UP( _d, _base ) ( ( (_d)/(_base) + 1)*(_base) )

/**********************************************************************
Code::Block : private functions
**********************************************************************/
static void printCacheInfo( Ptr_GCache This )
{
    printf("\n----Cache Control Block----\n");
    printf("Cache : \n");
    printf("Cache Node Size = %x(hex)\n", This->mClientInfo.mSzElem );
    printf("Cache Node Memory size = %x(hex)\n", PTR_GCCB_MEM(This)->mSzBytes );
    printf("\t Start From 0x%x\n", PTR_GCCB_MEM(This) );
    printf("\t Cache Start From 0x%x\n", PTR_GCCB_MEM(This)->pFirstNode );
    printf("\t Cache End at 0x%x \n", PTR_GCCB_MEM(This)->pLastNode );
    printf("\t Cache Insert at 0x%x\n", PTR_GCCB_MEM(This)->pCurInsertPos );
    printf("---------------------------\n");
}
/**********************************************************************
Code::Block : public functions
**********************************************************************/
bool createGCache
(
    Ptr_GCache This,
    const unsigned int aCacheSize,
    const Ptr_Client_Info aPtrClientInfo
)
{
    bool bSuccess;
    unsigned int actualCacheSize;

    bSuccess = false;
    actualCacheSize = MATH_ROUND_UP( aCacheSize - sizeof(GCCB), aPtrClientInfo->mSzElem ) + sizeof(GCCB);

    do
    {
        if( !This
                || ! aPtrClientInfo
                || 10*aPtrClientInfo->mSzElem + sizeof( GCCB ) > actualCacheSize
          )
        {
            break;
        }

        memset( (void*)This, 0, sizeof(GCache) );
        memcpy( &This->mClientInfo, aPtrClientInfo, sizeof( *aPtrClientInfo ) );

        This->pIntrl = malloc( actualCacheSize );
        memset( This->pIntrl, 0, actualCacheSize );

        if( !This->pIntrl ) break;

        ( (PGCCB)This->pIntrl )->pCacheMemory = This->pIntrl;
        ( (PGCCB)This->pIntrl )->mSzBytes =  actualCacheSize;
        ( (PGCCB)This->pIntrl )->pFirstNode = GET_NODE_START_ADDRESS( This->pIntrl );
        ( (PGCCB)This->pIntrl )->pLastNode = GET_NODE_END_ADDRESS( This->pIntrl );
        ( (PGCCB)This->pIntrl )->pCurInsertPos = GET_NODE_START_ADDRESS( This->pIntrl );
        bSuccess = true;
    }
    while(0);

    return bSuccess;

}

//!TO-DO: no-known error will occur at free()
void freeGCache( Ptr_GCache This )
{
    if ( NULL != This && NULL != This->pIntrl )
    {
        free( This->pIntrl );
        memset( This, 0, sizeof( GCache ) );
    }
}

/** \brief read a record from database with the given key
 *
 * \param aKeyWant
 <b>given key which should be unique by client </b>
 * \param aNodePtr
 <b>User should give the space to save read result from Cache
 * \return true if operation successful
 *
 */
bool readGCache
(
    const Ptr_GCache This,
    const Ptr_Cache_Node_Key aKeyWant,
    Ptr_Cache_Node aNodePtr
)
{
    CmpKeyAndNode compareFunction;
    ClientRead	clientRead;
    PGCCB pIntrl;
    void* visitPtr;
    bool rValue;
    unsigned int szNode;

    szNode = This->mClientInfo.mSzElem;
    visitPtr = NULL;
    rValue = false;
    compareFunction = NULL;
    pIntrl = PTR_GCCB_MEM( This );

    clientRead = This->mClientInfo.mClientRead;
    compareFunction = This->mClientInfo.mFuncKeyNodeCmp;

    assert( compareFunction );
    assert( clientRead );

    do
    {
        bool cacheHit = false;
        if( pIntrl->pCurInsertPos == pIntrl->pFirstNode )
        {
            Debug_Printf("Cache empty : read <datasource>-<cache>-<client>\n");
            clientRead( aKeyWant, pIntrl->pCurInsertPos );                 /**< read from datasource to cache */
            memcpy( aNodePtr, pIntrl->pCurInsertPos, szNode );          /**< read from cache to client */
            pIntrl->pCurInsertPos = (void*)( (char*)pIntrl->pCurInsertPos + szNode );
            rValue = true;
            break;
        }
        else if( pIntrl->pCurInsertPos == GET_NODE_END_ADDRESS(pIntrl) )
        {
            /**< Cache full : just free room for new record */
            Debug_Printf("Cache full : remove first node from cache\n");
            memmove( pIntrl->pFirstNode, (void*)( (char*)(pIntrl->pFirstNode)+szNode ), pIntrl->mSzBytes - szNode );
            pIntrl->pCurInsertPos = (void*)( (char*)pIntrl->pFirstNode + pIntrl->mSzBytes - szNode - sizeof(GCCB) );
            memset( pIntrl->pCurInsertPos, 0, szNode );         /**< keep all free memory be zero */
        }
        else
        {
            Debug_Printf("Cache need search\n");
        }

        for( visitPtr = pIntrl->pFirstNode;
                visitPtr != pIntrl->pCurInsertPos;
                visitPtr = (void*)( (char*)visitPtr + szNode )
           )
        {
            if( compareFunction(aKeyWant,visitPtr) )
            {
                Debug_Printf("Cache Hit : read from cache\n");
                memcpy( aNodePtr, visitPtr, szNode );
                cacheHit = true;
                break;
            }
        }

        if( !cacheHit )
        {
            Debug_Printf("Cache miss Hit : read <datasource>-<cache>-<client>\n");
            clientRead( aKeyWant, pIntrl->pCurInsertPos );
            memcpy( aNodePtr, pIntrl->pCurInsertPos, szNode );
        }

        pIntrl->pCurInsertPos = (void*)( (char*)pIntrl->pCurInsertPos + szNode );
        rValue = true;
    }
    while(0);

    return rValue;
}
