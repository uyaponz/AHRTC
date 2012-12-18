// -*- C++ -*-
/*!
 * @file  AHLogPlayer.cpp
 * @brief ${rtcParam.description}
 * @date $Date$
 *
 * $Id$
 */

#include "AHLogPlayer.h"

#include <cstdio>
#include <sys/time.h>
#include <string>

#include <opencv2/opencv.hpp>

static const double g_playSpeed = 1.0;

static FILE *fpPMlt = NULL;
static FILE *fpRMlt = NULL;
static FILE *fpPuck = NULL;
static double beginTime;

double gettimeofday_sec() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return (double)t.tv_sec + (double)t.tv_usec * 1e-6;
}

int readLogData(FILE *fp, CvPoint2D64f &pos) {
    double now = gettimeofday_sec();

    bool isFirst = true;
    while (true) {
        long fpPos = ftell(fp); if (fpPos==-1) return -1;

        double tm; CvPoint2D64f p;
        int ret = fscanf(fp, "%lf %lf %lf", &tm, &(p.x), &(p.y));
        if (ret == EOF) return -1;

        if (tm >= now-beginTime) {
            if (isFirst) pos = p;
            fseek(fp, fpPos, SEEK_SET);
            break;
        }

        isFirst = false;
        pos = p;
    }

    return 1;
}

bool openFileAsReadable(FILE **fp, std::string filename) {
    *fp = fopen(filename.c_str(), "r");
    return (!(*fp)) ? false : true;
}

void writePos(CvPoint2D64f pos, OutPort<TimedDoubleSeq> &data_outOut, TimedDoubleSeq &data_out) {
    coil::TimeValue tm = coil::gettimeofday() * g_playSpeed;
    data_out.tm.sec  = tm.sec();
    data_out.tm.nsec = tm.usec() * 1000;

    data_out.data[0] = pos.x;
    data_out.data[1] = pos.y;

    data_outOut.write();
}

// Module specification
// <rtc-template block="module_spec">
static const char* ahlogplayer_spec[] =
{
    "implementation_id", "AHLogPlayer",
    "type_name",         "AHLogPlayer",
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
    "conf.default.load_dir", "../common/log",
    // Widget
    "conf.__widget__.load_dir", "text",
    // Constraints
    ""
};
// </rtc-template>

/*!
 * @brief constructor
 * @param manager Maneger Object
 */
AHLogPlayer::AHLogPlayer(RTC::Manager* manager)
    // <rtc-template block="initializer">
    : RTC::DataFlowComponentBase(manager),
      m_puckXY_outOut("PuckXY_out", m_puckXY_out),
      m_malletXY_outOut("MalletXY_out", m_malletXY_out),
      m_armXY_outOut("ArmXY_out", m_armXY_out)

      // </rtc-template>
{
}

/*!
 * @brief destructor
 */
AHLogPlayer::~AHLogPlayer()
{
}



RTC::ReturnCode_t AHLogPlayer::onInitialize()
{
    // Registration: InPort/OutPort/Service
    // <rtc-template block="registration">
    // Set InPort buffers

    // Set OutPort buffer
    addOutPort("PuckXY_out", m_puckXY_outOut);
    addOutPort("MalletXY_out", m_malletXY_outOut);
    addOutPort("ArmXY_out", m_armXY_outOut);

    // Set service provider to Ports

    // Set service consumers to Ports

    // Set CORBA Service Ports

    // </rtc-template>

    // <rtc-template block="bind_config">
    // Bind variables and configuration variable
    bindParameter("load_dir", m_load_dir, "../common/log");

    // </rtc-template>

    m_malletXY_out.data.length(2);
    m_armXY_out.data.length(2);
    m_puckXY_out.data.length(2);

    m_malletXY_out.data[0] = m_malletXY_out.data[1] = 0.0;
    m_armXY_out.data[0]    = m_armXY_out.data[1]    = 0.0;
    m_puckXY_out.data[0]   = m_puckXY_out.data[1]   = 0.0;

    return RTC::RTC_OK;
}


RTC::ReturnCode_t AHLogPlayer::onFinalize()
{
    return RTC::RTC_OK;
}

/*
  RTC::ReturnCode_t AHLogPlayer::onStartup(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t AHLogPlayer::onShutdown(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/


RTC::ReturnCode_t AHLogPlayer::onActivated(RTC::UniqueId ec_id)
{
    std::string date;
    std::cout << "input log date (yyyyMMddHHmmss) > ";
    std::cin >> date;

    std::string basedir = m_load_dir + "/";

    if (!fpPMlt) {
        if (!openFileAsReadable(&fpPMlt, basedir+"LogPM_"+date+".log"))
            std::cerr << "cannot open file (LogPM)" << std::endl;
    }
    if (!fpRMlt) {
        if (!openFileAsReadable(&fpRMlt, basedir+"LogRM_"+date+".log"))
            std::cerr << "cannot open file (LogRM)" << std::endl;
    }
    if (!fpPuck) {
        if (!openFileAsReadable(&fpPuck, basedir+"LogPK_"+date+".log"))
            std::cerr << "cannot open file (LogPK)" << std::endl;
    }

    if (!fpPMlt && !fpRMlt && !fpPuck) {
        std::cerr << "FAILED TO OPEN LOGFILE" << std::endl;
        return RTC::RTC_ERROR;
    }

    beginTime = gettimeofday_sec();

    return RTC::RTC_OK;
}


RTC::ReturnCode_t AHLogPlayer::onDeactivated(RTC::UniqueId ec_id)
{
    if (fpPMlt) { fclose(fpPMlt); fpPMlt=NULL; }
    if (fpRMlt) { fclose(fpRMlt); fpRMlt=NULL; }
    if (fpPuck) { fclose(fpPuck); fpPuck=NULL; }

    return RTC::RTC_OK;
}


RTC::ReturnCode_t AHLogPlayer::onExecute(RTC::UniqueId ec_id)
{
    CvPoint2D64f pMltPos;
    int retPMlt = readLogData(fpPMlt, pMltPos);
    CvPoint2D64f rMltPos;
    int retRMlt = readLogData(fpRMlt, rMltPos);
    CvPoint2D64f pPuckPos;
    int retPuck = readLogData(fpPuck, pPuckPos);

    if (retPMlt >= 1) writePos(pMltPos,  m_malletXY_outOut, m_malletXY_out);
    if (retRMlt >= 1) writePos(rMltPos,  m_armXY_outOut,    m_armXY_out);
    if (retPuck >= 1) writePos(pPuckPos, m_puckXY_outOut,   m_puckXY_out);

    return RTC::RTC_OK;
}

/*
  RTC::ReturnCode_t AHLogPlayer::onAborting(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t AHLogPlayer::onError(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t AHLogPlayer::onReset(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t AHLogPlayer::onStateUpdate(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t AHLogPlayer::onRateChanged(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/



extern "C"
{

    void AHLogPlayerInit(RTC::Manager* manager)
    {
        coil::Properties profile(ahlogplayer_spec);
        manager->registerFactory(profile,
                                 RTC::Create<AHLogPlayer>,
                                 RTC::Delete<AHLogPlayer>);
    }

};


