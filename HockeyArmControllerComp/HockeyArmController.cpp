// -*- C++ -*-
/*!
 * @file  HockeyArmController.cpp
 * @brief ${rtcParam.description}
 * @date $Date$
 *
 * $Id$
 */

#include "HockeyArmController.h"

#include <iostream>
#include <cstdio>
#include <cstdlib>

#include <opencv2/opencv.hpp>

#include "HockeyDefinitions.hpp"
#include "PositionController.hpp"
#include "DeviceManager.hpp"
#include "MyMath.hpp"

/* --- グローバル変数 --- */
static unsigned char g_opened[2] = {0x00, 0x00}; // A-IOボードのオープンフラグ
PositionController g_posController[2]; // 位置決め制御クラスのインスタンス
// アームの初期位置
//const CvPoint2D64f g_initXY = {0.0, ARM_1LEN + ARM_2LEN - 50};
//const CvPoint2D64f g_initXY = {0.0, 280+203};
const CvPoint2D64f g_initXY = {0.0, 300+203};
const CvPoint2D64f g_initTh = getIK(g_initXY, ARM_1LEN, ARM_2LEN);


static void clearEncoderCountWithZ(PositionController &p1, PositionController &p2)
{
    p1.encoderInit(false); usleep(100000);
    p2.encoderInit(false); usleep(100000);
    fprintf(stderr, "Z clear OK!\n");

    p1.moveTo(INIT_ENC_X, 500);
    p2.moveTo(INIT_ENC_Y, 500);
//    while (!p1.isMoved() || !p2.isMoved()) { usleep(100); }
//
//    p1.servoOff(); p2.servoOff();
//    p1.encoderInit(true);
//    p2.encoderInit(true);
//    p1.moveTo(0, 300);
//    p2.moveTo(0, 300);
//    p1.servoOn();  p2.servoOn();

    fprintf(stderr, "Enc init OK!\n");
}

// Module specification
// <rtc-template block="module_spec">
static const char* hockeyarmcontroller_spec[] =
  {
    "implementation_id", "HockeyArmController",
    "type_name",         "HockeyArmController",
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
    "conf.default.arm_offset", "208.0",
    "conf.default.arm_len_1", "560.0",
    "conf.default.arm_len_2", "319.0",
    "conf.default.initial_enc_x", "1177",
    "conf.default.initial_enc_y", "1658",
    "conf.default.slow_limit_x", "5000",
    "conf.default.slow_limit_y", "5000",
    // Widget
    "conf.__widget__.arm_offset", "text",
    "conf.__widget__.arm_len_1", "text",
    "conf.__widget__.arm_len_2", "text",
    "conf.__widget__.initial_enc_x", "text",
    "conf.__widget__.initial_enc_y", "text",
    "conf.__widget__.slow_limit_x", "text",
    "conf.__widget__.slow_limit_y", "text",
    // Constraints
    ""
  };
// </rtc-template>

/*!
 * @brief constructor
 * @param manager Maneger Object
 */
HockeyArmController::HockeyArmController(RTC::Manager* manager)
    // <rtc-template block="initializer">
  : RTC::DataFlowComponentBase(manager),
    m_armXY_inIn("ArmXY_in", m_armXY_in),
    m_ArmXY_outOut("ArmXY_out", m_ArmXY_out),
    m_AHServicePort("AHService")

    // </rtc-template>
{
}

/*!
 * @brief destructor
 */
HockeyArmController::~HockeyArmController()
{
}



RTC::ReturnCode_t HockeyArmController::onInitialize()
{
  // Registration: InPort/OutPort/Service
  // <rtc-template block="registration">
  // Set InPort buffers
  addInPort("ArmXY_in", m_armXY_inIn);
  
  // Set OutPort buffer
  addOutPort("ArmXY_out", m_ArmXY_outOut);
  
  // Set service provider to Ports
  
  // Set service consumers to Ports
  m_AHServicePort.registerConsumer("AHCommonDataService", "AHCommon::AHCommonDataService", m_ahCommonDataService);
  
  // Set CORBA Service Ports
  addPort(m_AHServicePort);
  
  // </rtc-template>

  // <rtc-template block="bind_config">
  // Bind variables and configuration variable
  bindParameter("arm_offset", m_arm_offset, "208.0");
  bindParameter("arm_len_1", m_arm_len_1, "560.0");
  bindParameter("arm_len_2", m_arm_len_2, "319.0");
  bindParameter("initial_enc_x", m_initial_enc_x, "1177");
  bindParameter("initial_enc_y", m_initial_enc_y, "1658");
  bindParameter("slow_limit_x", m_slow_limit_x, "5000");
  bindParameter("slow_limit_y", m_slow_limit_y, "5000");
  
  // </rtc-template>

  /* --- コンポーネント入出力の初期化 --- */
  // 入力の初期化
  m_armXY_in.data.length(2);
  m_armXY_in.data[0] = g_initXY.x;
  m_armXY_in.data[1] = g_initXY.y;
  // 出力の初期化
  m_ArmXY_out.data.length(2);
  m_ArmXY_out.data[0] = g_initXY.x;
  m_ArmXY_out.data[1] = g_initXY.y;
  m_ArmXY_outOut.write();

  /* --- A-IOボードのオープン --- */
  if (openDevices(1, g_opened[0])) return RTC::RTC_ERROR;
  if (openDevices(2, g_opened[1])) return RTC::RTC_ERROR;

  /* --- 位置決め制御インスタンスの初期化 --- */
  
  // X軸
  g_posController[0].init(1, ENCODER_EVAL, SLOW_LIMIT_X*ENCODER_EVAL, 3000);
  g_posController[0].start(true);
  g_posController[0].startMove();
  g_posController[0].servoOn();
  // Y軸
  g_posController[1].init(2, ENCODER_EVAL, SLOW_LIMIT_Y*ENCODER_EVAL, 3000);
  g_posController[1].start(true);
  g_posController[1].startMove();
  g_posController[1].servoOn();

  usleep(500000);

  /* --- clear encoder count with Z --- */
  std::cerr << "!!!" << std::endl;
  clearEncoderCountWithZ(g_posController[0], g_posController[1]);

  /* --- アームを初期位置に移動する --- */
  g_posController[0].moveTo(rad2pulse(g_initTh.x - M_PI/2, 2500*ENCODER_EVAL) * MOTOR_X_RATIO + INIT_ENC_X, 300);
  g_posController[1].moveTo(rad2pulse(g_initTh.y         , 2500*ENCODER_EVAL) * MOTOR_Y_RATIO + INIT_ENC_Y, 300);
  fprintf(stderr, "ARM init OK!\n");

  return RTC::RTC_OK;
}


RTC::ReturnCode_t HockeyArmController::onFinalize()
{
  ;

  /* --- アームを停止位置(角度:(pi/2,0)[rad])に移動する --- */
  g_posController[0].moveTo(0 + INIT_ENC_X, 500);
  g_posController[1].moveTo(0 + INIT_ENC_Y, 500);
  while (!g_posController[0].isMoved() || !g_posController[1].isMoved()) { usleep(100); }

  fprintf(stderr, "ARM finalize OK!\n");

  /* --- 位置決め制御を停止する --- */
  g_posController[0].stop();
  g_posController[1].stop();

  /* --- A-IOボードをクローズする --- */
  closeDevices(1, g_opened[0]);
  closeDevices(2, g_opened[1]);

  return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t HockeyArmController::onStartup(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t HockeyArmController::onShutdown(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/


RTC::ReturnCode_t HockeyArmController::onActivated(RTC::UniqueId ec_id)
{
  ;

  /* --- 位置決め制御を開始する --- */
  g_posController[0].startMove(); g_posController[0].servoOn();
  g_posController[1].startMove(); g_posController[1].servoOn();

  return RTC::RTC_OK;
}


RTC::ReturnCode_t HockeyArmController::onDeactivated(RTC::UniqueId ec_id)
{
  ;

  /* --- 位置決め制御を停止する --- */
//  g_posController[0].stopMove();// g_posController[0].servoOff();
//  g_posController[1].stopMove();// g_posController[1].servoOff();

  return RTC::RTC_OK;
}


RTC::ReturnCode_t HockeyArmController::onExecute(RTC::UniqueId ec_id)
{
  ;

  // output the arm position
  static signed long prevArmX = 0;
  static signed long prevArmY = 0;
  signed long armX = g_posController[0].getEncCount();
  signed long armY = g_posController[1].getEncCount();
  if (armX != prevArmX  ||  armY != prevArmY) {
      coil::TimeValue tm(coil::gettimeofday());
      m_ArmXY_out.tm.sec  = tm.sec();
      m_ArmXY_out.tm.nsec = tm.usec() * 1000;

      CvPoint2D64f th = {
          pulse2rad((armX-INIT_ENC_X)/MOTOR_X_RATIO, 2500*ENCODER_EVAL) + M_PI/2.0,
          pulse2rad((armY-INIT_ENC_Y)/MOTOR_Y_RATIO, 2500*ENCODER_EVAL)
      };

      CvPoint2D64f xy = getFK(th, ARM_1LEN, ARM_2LEN);
      m_ArmXY_out.data[0] = xy.x;
      m_ArmXY_out.data[1] = xy.y - ARM_OFFSET - HKY_H/2.0;

      m_ArmXY_outOut.write();
  }

  if (!m_armXY_inIn.isNew()) return RTC::RTC_OK;
  m_armXY_inIn.read();

  CvPoint2D64f xy = {
      m_armXY_in.data[0],
      m_armXY_in.data[1] + ARM_OFFSET + HKY_H/2.0
  };
  if (!isArmMovable(xy, ARM_1LEN*0.97, ARM_2LEN*0.97, HKY_W, ARM_MLT_R*1.03, ARM_OFFSET))
      return RTC::RTC_OK;

  CvPoint2D64f th = getIK(xy, ARM_1LEN, ARM_2LEN, true);
  g_posController[0].moveTo(rad2pulse(th.x - M_PI/2, 2500*ENCODER_EVAL) * MOTOR_X_RATIO + INIT_ENC_X, 1000);
  g_posController[1].moveTo(rad2pulse(th.y         , 2500*ENCODER_EVAL) * MOTOR_Y_RATIO + INIT_ENC_Y, 800);

  //std::cout << m_armXY_in.data[0] << ", " << m_armXY_in.data[1] << std::endl;

  return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t HockeyArmController::onAborting(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t HockeyArmController::onError(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t HockeyArmController::onReset(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t HockeyArmController::onStateUpdate(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t HockeyArmController::onRateChanged(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/



extern "C"
{
 
  void HockeyArmControllerInit(RTC::Manager* manager)
  {
    coil::Properties profile(hockeyarmcontroller_spec);
    manager->registerFactory(profile,
                             RTC::Create<HockeyArmController>,
                             RTC::Delete<HockeyArmController>);
  }
  
};


