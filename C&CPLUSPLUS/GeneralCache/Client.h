#ifndef _CLENT_H_
#define _CLENT_H_

#include "GCache_Type.h"
#include "GCache_pub.h"

typedef struct _Cache_Elem
{
    char* _name;		//搞成一个26进制的数来表示
    int mAge;
    bool mSex;
} CACHE_ETYPE;

bool createTestData( unsigned int elemAmount );

bool freeTestData( );

bool findElem( const Ptr_Cache_Node_Key aKey, const Ptr_Cache_Node aNode );

bool ReadElem( const Ptr_Cache_Node_Key aKey, Ptr_Cache_Node aNode );

bool buildNameWithIdx( char* pName, int idx );

#endif
