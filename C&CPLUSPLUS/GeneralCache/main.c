#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "Client.h"
#include "GCache_pub.h"
#include "Common.h"

//cache使用的memory占所有数据memory大小的百分比
#define cache_rito (0.01f)

#define REGION_START {
#define REGION_END	}

/*<--Test Code-->*/

///@name Test_buildNameWithIdx
///@description test function for buildNameWithIdx( in_out pName : char*, in index : int)
bool Test_buildNameWithIdx()
{
    bool passed;
    int idx;

    /// start of test case
    int index_array[] =
    {
        1, 2, 3, 4,
        27 + 1,
        27*27*27 + 1, 27*27 , 27, 0,
        27*27*27 + 27*27 + 27 + 1,
        27*27*27 + 2*27*27 + 3*27 + 4,
        27*27*27*27 - 1
    };

    char* name_array[] =
    {
        "---A","---B","---C","---D",        /*1, 2, 3, 4,*/
        "--AA",                             /*27 + 1*/
        "A--A","-A--","--A-","----",
        "AAAA",
        "ABCD",
        "ZZZZ"
    };
    assert( Cnt_Of_Array( index_array ) == Cnt_Of_Array( name_array) );

    passed = true;

    for( idx = 0; idx < Cnt_Of_Array( index_array ); ++idx )
    {
        char pName[5];
        buildNameWithIdx( pName, index_array[idx] );
        passed &= !strncmp( pName, name_array[idx], 5 );
        if( !passed )
        {
            printf("%s test fail at %d test case\n", __FUNCTION__, idx + 1 );
            printf("--->function build name is : %s\n", pName);
            printf("--->name should be : %s\n", name_array[idx]);
            break;
        }
    }

    assert( passed );
    return passed;
}

///@name Test_createTestData
///@description test function for createTestData( in eleCount : unsigned int )
bool Test_createTestData()
{
    bool rValue;
    unsigned int dataSize;
    int index;

    rValue = true;
    dataSize = 27*27*27*27;
    createTestData( dataSize );

    extern CACHE_ETYPE* testData;

    CACHE_ETYPE* pVisitor = testData;

    for( index = 0; index < dataSize ; ++index, ++pVisitor )
    {
        char curName[5] = {0};
        buildNameWithIdx( curName, index );

        if( strncmp( curName, pVisitor->_name, 5 )
                || index != pVisitor->mAge
                || index %2 != pVisitor->mSex
          )
        {
            printf("Failed at %5dth record( %s, %d, %d)\n", index, pVisitor->_name, pVisitor->mAge, pVisitor->mSex );

            rValue = false;
            break;
        }
        //printf("Passed at %5dth record( %s, %d, %d)\n", index, pVisitor->_name, pVisitor->mAge, pVisitor->mSex );
    }

    freeTestData();

    return rValue;
}

///@name Test_ReadElem
///@description test function for ReadElem( in aKey : Ptr_Cache_Node_Key, in_out aNode: Ptr_Cache_Node )
bool Test_ReadElem( const bool isRandomTest )
{
    //variable definition block
    bool rValue;
    unsigned int szTestDb;
    int index;
    int testDataSet[] = {0, 1, 1000, 100000-1};
    int randomDataSet[1000];
    int testDataSetSize;

    //variable init block
    rValue = true;
    szTestDb = 100000;
    for( index = 0; index < 1000; ++index )
    {
        srand( index );
        randomDataSet[index] = rand() % szTestDb;
    }

    createTestData( szTestDb );

    //process
    int* pTestData = !isRandomTest ? testDataSet : randomDataSet;
    testDataSetSize = isRandomTest ? Cnt_Of_Array( randomDataSet ) : Cnt_Of_Array( testDataSet );

    for( index = 0; index < testDataSetSize; ++index )
    {
        char curname[5] = {0};
        CACHE_ETYPE readResult;

        buildNameWithIdx( curname, pTestData[index] );
        ReadElem( curname, &readResult );

        if( ReadElem( curname, &readResult ) &&
                ( strncmp( curname, readResult._name, 5 )
                  || readResult.mAge != pTestData[index]
                  || readResult.mSex != pTestData[index]%2
                )
          )
        {
            printf("Fail at %dth test case: (%s, %d, %d) != %s\n", index, readResult._name,readResult.mAge, readResult.mSex, curname );
            rValue = false;
            break;
        }
        //printf("Pass %s at %dth test case: (%s, %d, %d)\n", curname, index, readResult._name,readResult.mAge, readResult.mSex );
    }

    freeTestData();

    return rValue;
}

///@name Test_findElem
///@description test function for findElem( in aKey : const Ptr_Cache_Node_Key, in aNode :const Ptr_Cache_Node )
bool Test_findElem( )
{
    bool rValue;
    int index;
    char *pNameList[] = { "---A", "---E", "A--A" };
    CACHE_ETYPE testRecordList[] = { {"---A",1,1},{"---E",1,2},{"A--B",1,3} };
    bool resultWanted[] = { true, true, false };

    rValue = true;

    for( index = 0; index < Cnt_Of_Array(pNameList); ++index )
    {
        rValue &= ( resultWanted[index] == findElem( pNameList[index], testRecordList+index ) );
        if( !rValue )
        {
            printf("Failed at %dth : (%s, %d, %d)\n", index, testRecordList[index]._name, testRecordList[index].mAge, testRecordList[index].mSex );
            break;
        }
    }

    return rValue;
}

bool Test_readGCache( const bool isRandomTest )
{
    bool rValue;
    unsigned int dataSize;
    unsigned int cacheSize;
    int index;
    GCache testCacheObj;
    char* pTestList[] = { "----", "---A", "--A-", "AAAA" };
    char pTestListRandom[10000][5];
    int sizeofTestSet;

    rValue = true;
    memset( &testCacheObj, 0, sizeof testCacheObj );
    dataSize = 27*27*27*27;
    cacheSize = (unsigned int)( cache_rito*dataSize );

    //create random test data
    for( index = 0; index < 10000; ++index )
    {
        srand( index );
        int random = rand() % dataSize;

        buildNameWithIdx( pTestListRandom[index], random );
    }
    sizeofTestSet = isRandomTest ? Cnt_Of_Array( pTestListRandom ) : Cnt_Of_Array(pTestList);

    ///[1] build data structure
    ///[1.1] Create Test Data Base
    createTestData( dataSize );
    ///[1.2] Create GCache
    Client_Info pClientInfor = { sizeof(CACHE_ETYPE), NULL, findElem, ReadElem };
    createGCache( &testCacheObj, cacheSize, &pClientInfor );

    for( index = 0; index < sizeofTestSet; ++index )
    {
        char* keyWant = isRandomTest ? pTestListRandom[index] : pTestList[index];
        CACHE_ETYPE recordWanted;
        readGCache( &testCacheObj, keyWant, &recordWanted );

        if( strncmp( keyWant, recordWanted._name, 5 ) )
        {
            printf("Faild at testData[%d]->%s VS (%s,%d,%d)",index,keyWant, recordWanted._name, recordWanted.mAge, recordWanted.mSex );
            rValue = false;
            break;
        }

        //printf("Passed test %d [%s == %s]\n", index, keyWant, recordWanted._name );
    }

    freeGCache( &testCacheObj );
    freeTestData();

    return rValue;
}

int main(int argc, char const *argv[])
{
    //测试从27进制数的转换是否正确，我用27进制数来对每一个记录编号，作为记录的唯一识别标志
    //assert( Test_buildNameWithIdx() );                      /**< Passed */

    //测试创建测试使用的数据是否正常
    //assert( Test_createTestData() );                        /**< Passed */

    //测试client端提供的读
    //assert( Test_ReadElem( false ) );                       /**< Passed */

    //测试客户端提供的读：随机测试
    //assert( Test_ReadElem( true ) );                        /**< Passed */

    //测试客户端提供的key-racord验证函数
    //assert( Test_findElem() );                              /**< Passed */

    //测试从GCache中读取数据
    assert( Test_readGCache( true ) );                      /**< Fail TODO */

    printf("Pass All Test\n");

    return 0;
}
