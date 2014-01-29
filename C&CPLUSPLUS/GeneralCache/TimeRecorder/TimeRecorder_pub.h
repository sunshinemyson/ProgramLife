#ifndef TIMERECORDER_PUB_H_INCLUDED
#define TIMERECORDER_PUB_H_INCLUDED

#include "..\Common.h"

//! 注册到时间记录器的客户端ID。
/*! More detailed enum description. */
typedef unsigned int ClientID;enum{
    ENU_GCACHE,         /*!< General Cacher identify  */
    ENU_IME_ENG,        /*!< IME Engine identify */

    ENU_CNT
};

//! 使用时间记录器时，客户端可以指定每次log时间的类型
typedef struct time_class_map{
    unsigned int tClassID;      /*!< 数字标示符 */
    char*        pClassName;    /*!< 类型名称(例如,可以指定记录启动时间，执行时间，停止时间，等待时间)   */
    unsigned int szClassName;   /*!< 类型名称长度 */
}TIME_CLASS_MAP;

/////////////////////////////////////////////////
/// \brief Public Interface to start time record for given client.
///
/// \param aClientID the user identify
/// \return true start operation done else false
/////////////////////////////////////////////////
bool startTimeRecorder( const ClientID aClientID );

/////////////////////////////////////////////////
/// \brief Public Interface to start time recorder with time class modifier
///
/// \param aClientID client identify
/// \param aMap time class map
/// \param len total length of aMap
/// \return true start operation done else false
///
/////////////////////////////////////////////////
bool startTimeRecorderWithClass
    (
     const ClientID aClientID
     , const TIME_CLASS_MAP *const aTCMap
     , unsigned int len
    );

/////////////////////////////////////////////////
/// \brief Public Interface to stop time recorder with client identify
///
/// \param aClientID client identify
/// \return true stop success else false
///
/////////////////////////////////////////////////
bool stopTimerRecorder( const ClientID aClientID );

/////////////////////////////////////////////////
/// \brief
///
/// \param
/// \param
/// \return
///
/////////////////////////////////////////////////
bool logTime( const ClientID, unsigned int aTime );

/////////////////////////////////////////////////
/// \brief client log time with class of time
///
/// \param aClientID client identify
/// \param aTCID time class id
/// \return true log success else false
///
/////////////////////////////////////////////////
bool logTimeClass( const ClientID aClientID, unsigned int aTCID, unsigned int aTime );

/////////////////////////////////////////////////
/// \brief create time report for client
///
/// \param aClientID client identify
/// \param aRpt if NULL, report will be show in stdout, allocate
/// memory for report and return it to client
/// \return true if build success else false
///
/////////////////////////////////////////////////
bool buildReport( const ClientID aClientID, char* aRpt );

#endif // TIMERECORDER_PUB_H_INCLUDED
