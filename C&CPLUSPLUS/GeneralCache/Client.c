#include "Client.h"
#include "GCache_pub.h"

#include "stdio.h"
#include "string.h"
#include "stdlib.h"

typedef CACHE_ETYPE* P_CACHE_ETYPE;

P_CACHE_ETYPE testData = NULL;
unsigned int testSize = 0;
char* pCharSet = "-ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const int name_Len = 5;
static bool hasInited = false;

bool buildNameWithIdx( char* pName, int idx );

/*
	@ data: testData，被动态创建
	@ elemSize : 测试数据的大小
*/
bool createTestData( unsigned int elemAmount )
{
    unsigned int idx;
    //const int name_Len = 4;
    if( !hasInited )
    {
        testSize = elemAmount;
        testData = (P_CACHE_ETYPE)malloc( sizeof(CACHE_ETYPE)*elemAmount );

        for( idx = 0; idx < elemAmount; ++idx )
        {
            (testData+idx)->_name = (char*)malloc( sizeof(char)*name_Len );
            buildNameWithIdx( (testData+idx)->_name, idx );
            (testData+idx)->mAge = idx;
            (testData+idx)->mSex = idx%2;
        }
        hasInited = true;
    }

    return testData != NULL;
}

bool freeTestData()
{
    bool rVal;

    rVal = false;
    if( hasInited )
    {
        CACHE_ETYPE* iter = testData;
        CACHE_ETYPE* end = testData + testSize;
        while( iter != end )
        {
            free( iter->_name );
            iter++;
        }

        free( testData );
        testData = NULL;
        rVal = true;
        hasInited = false;
        testSize = 0;
    }

    return rVal;
}

bool findElem( const Ptr_Cache_Node_Key aKey, const Ptr_Cache_Node aNode )
{
    char* pKey;
    P_CACHE_ETYPE pNode;

    pKey = (char*) aKey;
    pNode = ( P_CACHE_ETYPE )aNode;

    return !strncmp( pKey, pNode->_name, name_Len );
}

bool ReadElem( const Ptr_Cache_Node_Key aKey, Ptr_Cache_Node aNode )
{
    char* pName;
    unsigned int idx;

    pName = (char*)aKey;

    for (idx = 0; idx < testSize; ++idx)
    {
        if( strcmp(pName, (testData+idx)->_name) == 0 )
        {
            memcpy( aNode, testData+idx, sizeof( CACHE_ETYPE ) );
            break;
        }
    }

    return idx != testSize;
}

bool buildNameWithIdx( char* pName, int idx )
{
    int i;
    int base[4] = { 27*27*27, 27*27, 27, 1 };

    memset( pName, '-', name_Len );
    pName[name_Len-1] = '\0';
    //"----\0"

    i = 0;
    while( idx && i < name_Len - 1 )
    {
        pName[i] = pCharSet[ idx/base[i] ];
        idx = idx%base[i];
        i++;
    }

    return true;
}
