// -*- C++ -*-
/*!
 * @file  AHViewer.cpp
 * @brief ${rtcParam.description}
 * @date $Date$
 *
 * $Id$
 */

#include "AHViewer.h"

#include "MyMath.hpp"
#include "HockeyDefinitions.hpp"

#include <opencv2/opencv.hpp>

static const char winName[] = "AHViewer";
static const int winW = 430;
static const int winH = 680;
static const int ofsX = 10;
static const int ofsY = 10;
static const double scale = 3.5;

void drawHockeyBoard(cv::Mat &f)
{
    { // 外枠 + 中央線
        CvPoint from = convH2CV(cvPoint2D64f(-HKY_W/2.0, -HKY_H/2.0),
                                cvPoint2D64f(HKY_W, HKY_H),
                                scale, cvPoint(ofsX,ofsY));
        CvPoint to   = convH2CV(cvPoint2D64f(HKY_W/2.0, HKY_H/2.0),
                                cvPoint2D64f(HKY_W, HKY_H),
                                scale, cvPoint(ofsX,ofsY));
        cv::rectangle(f, from, to, cv::Scalar(255,0,255));
        from = convH2CV(cvPoint2D64f(-HKY_W/2.0, 0.0),
                        cvPoint2D64f(HKY_W, HKY_H),
                        scale, cvPoint(ofsX,ofsY));
        to   = convH2CV(cvPoint2D64f(HKY_W/2.0, 0.0),
                        cvPoint2D64f(HKY_W, HKY_H),
                        scale, cvPoint(ofsX,ofsY));
        cv::line(f, from, to, cv::Scalar(255,0,255));
    }

    { // 赤線
        for (int i=0; i<6; i++) { // 横方向の線
            CvPoint from = convH2CV(cvPoint2D64f(-HKY_W/2.0, HKY_H/2.0 - 400.0*i),
                                    cvPoint2D64f(HKY_W, HKY_H),
                                    scale, cvPoint(ofsX,ofsY));
            CvPoint to = convH2CV(cvPoint2D64f(HKY_W/2.0, HKY_H/2.0 - 400.0*i),
                                  cvPoint2D64f(HKY_W, HKY_H),
                                  scale, cvPoint(ofsX,ofsY));
            cv::line(f, from, to, cv::Scalar(0,0,255), 1);
        }
        for (int i=-2; i<=2; i++) { // 縦方向の線
            CvPoint from = convH2CV(cvPoint2D64f(300.0*i, -HKY_H/2.0),
                                    cvPoint2D64f(HKY_W, HKY_H),
                                    scale, cvPoint(ofsX,ofsY));
            CvPoint to = convH2CV(cvPoint2D64f(300.0*i, HKY_H/2.0),
                                  cvPoint2D64f(HKY_W, HKY_H),
                                  scale, cvPoint(ofsX,ofsY));
            cv::line(f, from, to, cv::Scalar(0,0,255), 1);
        }
    }
}

void drawMallet(cv::Mat &f, CvPoint2D64f malletXY, double R, CvScalar color)
{
    CvPoint center = convH2CV(malletXY, cvPoint2D64f(HKY_W,HKY_H),
                              scale, cvPoint(ofsX,ofsY));
    int r_ = static_cast<int>((R/2.0) / scale);
    cv::circle(f, center, r_, color, -1);
}

// Module specification
// <rtc-template block="module_spec">
static const char* ahviewer_spec[] =
{
    "implementation_id", "AHViewer",
    "type_name",         "AHViewer",
    "description",       "${rtcParam.description}",
    "version",           "1.0.0",
    "vendor",            "ysatoh",
    "category",          "Category",
    "activity_type",     "PERIODIC",
    "kind",              "DataFlowComponent",
    "max_instance",      "1",
    "language",          "C++",
    "lang_type",         "compile",
    ""
};
// </rtc-template>

/*!
 * @brief constructor
 * @param manager Maneger Object
 */
AHViewer::AHViewer(RTC::Manager* manager)
    // <rtc-template block="initializer">
    : RTC::DataFlowComponentBase(manager),
      m_puckXY_inIn("PuckXY_in", m_puckXY_in),
      m_malletXY_inIn("MalletXY_in", m_malletXY_in),
      m_armXY_inIn("ArmXY_in", m_armXY_in),
      frame(cv::Size(winW,winH), CV_8UC3, cv::Scalar(255,255,255))

      // </rtc-template>
{
}

/*!
 * @brief destructor
 */
AHViewer::~AHViewer()
{
}



RTC::ReturnCode_t AHViewer::onInitialize()
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

    m_malletXY_in.data.length(2);
    m_armXY_in.data.length(2);
    m_puckXY_in.data.length(2);

    m_malletXY_in.data[0] = m_malletXY_in.data[1] = 0.0;
    m_armXY_in.data[0]    = m_armXY_in.data[1]    = 0.0;
    m_puckXY_in.data[0]   = m_puckXY_in.data[1]   = 0.0;

    cv::namedWindow(winName, CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
    cv::imshow(winName, frame);
    cv::waitKey(10); cv::waitKey(10);

    return RTC::RTC_OK;
}


RTC::ReturnCode_t AHViewer::onFinalize()
{
    cv::destroyAllWindows();
    return RTC::RTC_OK;
}

/*
  RTC::ReturnCode_t AHViewer::onStartup(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t AHViewer::onShutdown(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/


RTC::ReturnCode_t AHViewer::onActivated(RTC::UniqueId ec_id)
{
    return RTC::RTC_OK;
}


RTC::ReturnCode_t AHViewer::onDeactivated(RTC::UniqueId ec_id)
{
    return RTC::RTC_OK;
}


RTC::ReturnCode_t AHViewer::onExecute(RTC::UniqueId ec_id)
{
    m_malletXY_inIn.read();
    m_armXY_inIn.read();
    m_puckXY_inIn.read();

    CvPoint2D64f pMlt = cvPoint2D64f(m_malletXY_in.data[0], m_malletXY_in.data[1]);
    CvPoint2D64f rMlt = cvPoint2D64f(m_armXY_in.data[0],    m_armXY_in.data[1]);
    CvPoint2D64f puck = cvPoint2D64f(m_puckXY_in.data[0],    m_puckXY_in.data[1]);

    frame = cv::Scalar(255,255,255);
    drawHockeyBoard(frame);
    drawMallet(frame, pMlt, PLAYER_MLT_R, cv::Scalar(255,128,0));
    drawMallet(frame, rMlt, ARM_MLT_R,    cv::Scalar(255,0,0));
    drawMallet(frame, puck, PUCK_R,       cv::Scalar(0,0,255));

    cv::imshow(winName, frame);
    cv::waitKey(10);

    return RTC::RTC_OK;
}

/*
  RTC::ReturnCode_t AHViewer::onAborting(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t AHViewer::onError(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t AHViewer::onReset(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t AHViewer::onStateUpdate(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t AHViewer::onRateChanged(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/



extern "C"
{

    void AHViewerInit(RTC::Manager* manager)
    {
        coil::Properties profile(ahviewer_spec);
        manager->registerFactory(profile,
                                 RTC::Create<AHViewer>,
                                 RTC::Delete<AHViewer>);
    }

};


