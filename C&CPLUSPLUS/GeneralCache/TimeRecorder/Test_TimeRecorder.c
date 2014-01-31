#include "Test_TimeRecorder.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

bool Test_TR_BlackBox( void ){
    unsigned int iClient;

    typedef unsigned int TTimeClass;enum{
        client_init_proc,
        client_work_proc,
        client_deinit_proc,

        client_cnt
    };

    srand( __LINE__ );
    iClient = rand()%100;
    const TIME_CLASS_MAP tMap[] =
        {
            { client_init_proc, "client_init_proc", strlen( "client_init_proc" ) },
            { client_work_proc, "client_work_proc", strlen( "client_work_proc" ) },
            { client_deinit_proc, "client_deinit_proc", strlen( "client_deinit_proc" ) }
        };

    typedef struct _testDataSet{
        TTimeClass mWhich;
        unsigned int mLog;
    }TDataSet;

    TDataSet testDataSet[] = {
        {
            client_init_proc, __LINE__
        },
        {
            client_work_proc, __LINE__
        },
        {
            client_deinit_proc, __LINE__
        }
    };

    COMPILE_ASSERT( Cnt_Of_Array( tMap ) == client_cnt );

    assert( startTimeRecorderWithClass(iClient, tMap, Cnt_Of_Array(tMap) ) );

    assert( logTimeClass( iClient, client_init_proc,testDataSet[client_init_proc].mLog ) );

    assert( logTimeClass( iClient, client_work_proc, testDataSet[client_work_proc].mLog ) );

    assert( logTimeClass( iClient, client_deinit_proc, testDataSet[client_deinit_proc].mLog ) );

    assert( stopTimerRecorder( iClient ) );

    buildReport( iClient, NULL );

    return true;
}
