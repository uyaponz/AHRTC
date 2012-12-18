// -*- C++ -*-
/*!
 * @file  AHLogger.cpp
 * @brief ${rtcParam.description}
 * @date $Date$
 *
 * $Id$
 */

#include "AHLogger.h"

#include <cstdio>
#include <string>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>

static FILE *fpPMlt = NULL;
static FILE *fpRMlt = NULL;
static FILE *fpPuck = NULL;
static double beginTime = 0.0;

double gettimeofday_sec()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return (double)t.tv_sec + (double)t.tv_usec * 1e-6;
}

std::string getLocalTimeAsString()
{
    time_t timer = time(NULL);
    struct tm *local = localtime(&timer);

    char tmp[256];
    sprintf(tmp,
            "%04d" "%02d" "%02d" "%02d" "%02d" "%02d",
            local->tm_year + 1900,
            local->tm_mon  + 1,
            local->tm_mday,
            local->tm_hour,
            local->tm_min,
            local->tm_sec);

    return std::string(tmp);
}

void writeLogData(FILE *fp,
                  InPort<TimedDoubleSeq> &data_inIn,
                  TimedDoubleSeq &data_in)
{
    if (fp == NULL) return;

    if (!data_inIn.isNew()) return;
    data_inIn.read();

    double now = gettimeofday_sec();
    fprintf(fp, "%lf %lf %lf¥n",
            now - beginTime, data_in.data[0], data_in.data[1]);
}

void writePMallet(InPort<TimedDoubleSeq> &data_inIn, TimedDoubleSeq &data_in) {
    writeLogData(fpPMlt, data_inIn, data_in);
}
void writeRMallet(InPort<TimedDoubleSeq> &data_inIn, TimedDoubleSeq &data_in) {
    writeLogData(fpRMlt, data_inIn, data_in);
}
void writePuck(InPort<TimedDoubleSeq> &data_inIn, TimedDoubleSeq &data_in) {
    writeLogData(fpPuck, data_inIn, data_in);
}

// Module specification
// <rtc-template block="module_spec">
static const char* ahlogger_spec[] =
{
    "implementation_id", "AHLogger",
    "type_name",         "AHLogger",
    "description",       "${rtcParam.description}",
    "version",           "1.0.0",
    "vendor",            "ysatoh",
    "category",          "Category",
    "activity_type",     "PERIODIC",
    "kind",              "DataFlowComponent",
    "max_instance",      "1",
    "language",          "C++",
    "lang_type",         "compile",
    // Configuration variables
    "conf.default.save_dir", "../common/log",
    // Widget
    "conf.__widget__.save_dir", "text",
    // Constraints
    ""
};
// </rtc-template>

/*!
 * @brief constructor
 * @param manager Maneger Object
 */
AHLogger::AHLogger(RTC::Manager* manager)
    // <rtc-template block="initializer">
    : RTC::DataFlowComponentBase(manager),
      m_puckXY_inIn("PuckXY_in", m_puckXY_in),
      m_malletXY_inIn("MalletXY_in", m_malletXY_in),
      m_armXY_inIn("ArmXY_in", m_armXY_in)

      // </rtc-template>
{
}

/*!
 * @brief destructor
 */
AHLogger::~AHLogger()
{
}



RTC::ReturnCode_t AHLogger::onInitialize()
{
    // Registration: InPort/OutPort/Service
    // <rtc-template block="registration">
    // Set InPort buffers
    addInPort("PuckXY_in", m_puckXY_inIn);
    addInPort("MalletXY_in", m_malletXY_inIn);
    addInPort("ArmXY_in", m_armXY_inIn);

    // Set OutPort buffer

    // Set service provider to Ports

    // Set service consumers to Ports

    // Set CORBA Service Ports

    // </rtc-template>

    // <rtc-template block="bind_config">
    // Bind variables and configuration variable
    bindParameter("save_dir", m_save_dir, "../common/log");

    // </rtc-template>

    m_puckXY_in.data.length(2);
    m_malletXY_in.data.length(2);
    m_armXY_in.data.length(2);
    m_puckXY_in.data[0]   = m_puckXY_in.data[1]   = 0.0;
    m_malletXY_in.data[0] = m_malletXY_in.data[1] = 0.0;
    m_armXY_in.data[0]    = m_armXY_in.data[1]    = 0.0;

    return RTC::RTC_OK;
}


RTC::ReturnCode_t AHLogger::onFinalize()
{
    return RTC::RTC_OK;
}

/*
  RTC::ReturnCode_t AHLogger::onStartup(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t AHLogger::onShutdown(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/


RTC::ReturnCode_t AHLogger::onActivated(RTC::UniqueId ec_id)
{
    std::string basepath  = m_save_dir + "/";
    std::string localtime = getLocalTimeAsString();

    if (!fpPMlt) fpPMlt=fopen((basepath + "LogPM_" + localtime + ".log").c_str(), "w");
    if (!fpRMlt) fpRMlt=fopen((basepath + "LogRM_" + localtime + ".log").c_str(), "w");
    if (!fpPuck) fpPuck=fopen((basepath + "LogPK_" + localtime + ".log").c_str(), "w");

    beginTime = gettimeofday_sec();

    return RTC::RTC_OK;
}


RTC::ReturnCode_t AHLogger::onDeactivated(RTC::UniqueId ec_id)
{
    if (fpPMlt) { fclose(fpPMlt); fpPMlt=NULL; }
    if (fpRMlt) { fclose(fpRMlt); fpRMlt=NULL; }
    if (fpPuck) { fclose(fpPuck); fpPuck=NULL; }

    return RTC::RTC_OK;
}


RTC::ReturnCode_t AHLogger::onExecute(RTC::UniqueId ec_id)
{
    writePMallet(m_malletXY_inIn, m_malletXY_in);
    writeRMallet(m_armXY_inIn,    m_armXY_in);
    writePuck   (m_puckXY_inIn,   m_puckXY_in);

    return RTC::RTC_OK;
}

/*
  RTC::ReturnCode_t AHLogger::onAborting(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t AHLogger::onError(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t AHLogger::onReset(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t AHLogger::onStateUpdate(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t AHLogger::onRateChanged(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/



extern "C"
{

    void AHLoggerInit(RTC::Manager* manager)
    {
        coil::Properties profile(ahlogger_spec);
        manager->registerFactory(profile,
                                 RTC::Create<AHLogger>,
                                 RTC::Delete<AHLogger>);
    }

};


