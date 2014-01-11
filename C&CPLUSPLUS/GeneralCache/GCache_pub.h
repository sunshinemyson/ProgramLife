#ifndef _GCACHE_PUB_H_
#define _GCACHE_PUB_H_

#include "GCache_Type.h"

typedef void* Ptr_Cache_Node;

typedef void* Ptr_Cache_Node_Key;

// 调用者需要指定的缓存比较函数
typedef bool (*Cmp2Node)( const void*, const void* );

typedef bool (*CmpKeyAndNode)( const Ptr_Cache_Node_Key,  const Ptr_Cache_Node );

// 用户需要指定一个读取数据的操作，在缓存没有命中的时候读取数据到cache中
typedef bool (*ClientRead)( const Ptr_Cache_Node_Key, Ptr_Cache_Node );

typedef struct _ClientInfo
{
    unsigned int mSzElem; 			///!每一个缓冲元素的大小，不支持大小不同的元素
    Cmp2Node mFuncNodeCmp;			///!用户需要提供如何比较两个Element
    CmpKeyAndNode mFuncKeyNodeCmp;	///!用户提供，用于判断当前key和node中的key是否一样
    ClientRead mClientRead;			///!用户提供，从数据库中读取一个数据
} Client_Info,*Ptr_Client_Info;

typedef struct _GCache
{
    void* pIntrl;		///!Cache控制块，放在动态分配的内存上
    Client_Info mClientInfo;	///!用户需要提供的信息
} GCache,*Ptr_GCache;

//创建一个"指定大小"的GCache
bool createGCache
(
    Ptr_GCache This,
    const unsigned int aCacheSize,
    const Ptr_Client_Info aClientInfo
);

void freeGCache( Ptr_GCache This );

//暂时不会实现
bool createGCacheWithClientMemory
(
    const Ptr_GCache This,
    void* pClientMemory,
    const unsigned int aCacheSize
);

//需要调用者给出CacheNode的key，我们才能从整个Cache中找到它，这时需要用到用户提供的比较函数
bool readGCache
(
    const Ptr_GCache This,
    const Ptr_Cache_Node_Key aKeyWant,
    Ptr_Cache_Node aNodePtr
);

//暂未实现
bool writeGCache
(
    const Ptr_GCache This,
    const Ptr_Cache_Node aNode
);

#endif
