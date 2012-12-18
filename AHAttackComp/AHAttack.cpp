// -*- C++ -*-
/*!
 * @file  AHAttack.cpp
 * @brief ${rtcParam.description}
 * @date $Date$
 *
 * $Id$
 */

#include "AHAttack.h"

#include "HockeyDefinitions.hpp"
#include "MyMath.hpp"

#include <deque>
#include <cmath>

static const double ARM_BASE_Y  = 170.0; // アーム待機Y座標
static const double ARM_GUARD_Y = 170.0;
static const double MEM_MIN_R   = 25.0;  // パック軌道を記憶する最小距離差
static const double MEM_MAX_R   = 500.0; // パック軌道を記憶する最大距離差
static const int    MEM_MAX_NUM = 300;   // 軌道を記憶する点数
static const int    USE_MEM_NUM = 7;    // 傾きを求めるために使うデータの点数

static int getProcNum(double puckY) {
    static const double min0 = HKY_H / 2.0 + 400.0 + 200.0;
    static const double min1 = HKY_H / 2.0 + 400.0;
    static const double min2 = HKY_H / 2.0;
    static const double min3 = ARM_BASE_Y + 350.0;
    static const double min4 = 0.0;

    if (min0 <= puckY) return 0;
    if (min1 <= puckY) return 1;
    if (min2 <= puckY) return 2;
    if (min3 <= puckY) return 3;
    if (min4 <= puckY) return 4;

    return 4;
}

// Module specification
// <rtc-template block="module_spec">
static const char* ahattack_spec[] =
{
    "implementation_id", "AHAttack",
    "type_name",         "AHAttack",
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
AHAttack::AHAttack(RTC::Manager* manager)
    // <rtc-template block="initializer">
    : RTC::DataFlowComponentBase(manager),
      m_puckXY_inIn("PuckXY_in", m_puckXY_in),
      m_malletXY_inIn("MalletXY_in", m_malletXY_in),
      m_armXY_inIn("ArmXY_in", m_armXY_in),
      m_armXY_outOut("ArmXY_out", m_armXY_out)

      // </rtc-template>
{
}

/*!
 * @brief destructor
 */
AHAttack::~AHAttack()
{
}



RTC::ReturnCode_t AHAttack::onInitialize()
{
    // Registration: InPort/OutPort/Service
    // <rtc-template block="registration">
    // Set InPort buffers
    addInPort("PuckXY_in", m_puckXY_inIn);
    addInPort("MalletXY_in", m_malletXY_inIn);
    addInPort("ArmXY_in", m_armXY_inIn);

    // Set OutPort buffer
    addOutPort("ArmXY_out", m_armXY_outOut);

    // Set service provider to Ports

    // Set service consumers to Ports

    // Set CORBA Service Ports

    // </rtc-template>

    /* Input */
    m_puckXY_in.data.length(0);
    m_malletXY_in.data.length(0);
    m_armXY_in.data.length(0);

    /* Output */
    m_armXY_out.data.length(2);
    m_armXY_out.data[0] = 0.0;
    m_armXY_out.data[1] = ARM_BASE_Y - HKY_H/2.0;
    m_armXY_outOut.write();

    return RTC::RTC_OK;
}

/*
  RTC::ReturnCode_t AHAttack::onFinalize()
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t AHAttack::onStartup(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t AHAttack::onShutdown(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/


RTC::ReturnCode_t AHAttack::onActivated(RTC::UniqueId ec_id)
{
    return RTC::RTC_OK;
}


RTC::ReturnCode_t AHAttack::onDeactivated(RTC::UniqueId ec_id)
{
    return RTC::RTC_OK;
}


RTC::ReturnCode_t AHAttack::onExecute(RTC::UniqueId ec_id)
{
    /* ポートから入力 */
    if (!m_puckXY_inIn.isNew() && !m_armXY_inIn.isNew()) return RTC::RTC_OK;
    m_puckXY_inIn.read(); m_armXY_inIn.read();
    if (m_puckXY_in.data.length()==0 || m_armXY_in.data.length()==0) return RTC::RTC_OK;

    double realArmX = m_armXY_in.data[0];
    double realArmY = m_armXY_in.data[1] + HKY_H/2.0;

    /* パックの軌道を記憶する */
    static std::deque<MyPos> recentPos;
    double x = m_puckXY_in.data[0];
    double y = m_puckXY_in.data[1] + HKY_H/2.0;
    if (recentPos.size() <= 0) { // データを1つも記憶していない場合は記憶して終了
        addDeque<MyPos>(recentPos, myPos(x,y), MEM_MAX_NUM);
        return RTC::RTC_OK;
    }
    else { // 前回入力した座標に近い場合は記憶しない
        MyPos front = recentPos.front();
        double dist = pow(x - front.x, 2) + pow(y - front.y, 2);
        double minR = MEM_MIN_R * MEM_MIN_R;
//        double maxR = MEM_MAX_R * MEM_MAX_R;
        if (minR < dist) {// && dist < maxR) {
            addDeque<MyPos>(recentPos, myPos(x,y), MEM_MAX_NUM);
        }
    }

    /* 領域 1〜4 のどこにいるのか判定する */
    static int prevProc = 0;
    int nowProc = getProcNum(y);
    /* 処理番号の遷移 */
    if      (prevProc==0 && nowProc==1) nowProc = 1; // 0 -> 1 = 1
    else if (prevProc==1 && nowProc==2) nowProc = 2; // 1 -> 2 = 2
    else if (prevProc==2 && nowProc==3) nowProc = 3; // 2 -> 3 = 3
    else if (prevProc==3 && nowProc==4) nowProc = 4; // 3 -> 4 = 4
    else if (prevProc==3 && nowProc==0) nowProc = 0; // 3 -> 0 = 0
    else if (prevProc==4 && nowProc==0) nowProc = 0; // 4 -> 0 = 0
    else                                nowProc = prevProc;

    // 2 or 3 : 軌道から直線を求める(XとYを逆にして計算する)
    static MyPos prevSlope = myPos(0.0, 0.0);
    static bool  doShot    = false;
    if (nowProc==2 || nowProc==3) {
        MyPos lineAB = calcSlope(recentPos, USE_MEM_NUM, true);
        if (lineAB.x > 0 || lineAB.x < 0) {
            double A=lineAB.x, B=lineAB.y;
            double armX = A * ARM_BASE_Y + B;

            // 出力に書き込む
            if (-670.0 < armX && armX < 670.0) {
                m_armXY_out.data[0] = armX;
                m_armXY_out.data[1] = ARM_BASE_Y - HKY_H/2.0;
                doShot = true;
                m_armXY_outOut.write();
                prevSlope = lineAB;
            }
        }
        else {
            doShot = false;
        }
    }

    // 3 -> 4 : 打ちに行く
    if ((nowProc==3 || nowProc==4) && doShot) {
        double distPM = sqrt(pow(realArmX-x, 2) + pow(realArmY-y, 2));
        if (distPM <= 350.0) {
            double A  = prevSlope.x;
            double th = -atan(A) + M_PI/2.0;
            double l  = 300.0;
            double vecX = l * cos(th);
            double vecY = l * sin(th);

//            CvPoint2D64f xy = {vecX, vecY + ARM_OFFSET + HKY_H/2.0};
//            if (!isArmMovable(xy, ARM_1LEN*0.97, ARM_2LEN*0.97, HKY_W, ARM_MLT_R*1.03, ARM_OFFSET)) {
                m_armXY_out.data[0] += vecX;
                m_armXY_out.data[1] += vecY;
//            }
//            else {
//                m_armXY_out.data[1] = (ARM_BASE_Y + 300.0) - HKY_H/2.0;
//            }
            m_armXY_outOut.write();
            doShot  = false;
        }
    }
    // (3 or 4) -> 0 : 戻す
    else if ((prevProc==3 || prevProc==4) && nowProc==0) {
        m_armXY_out.data[0] = 0.0;
        m_armXY_out.data[1] = ARM_BASE_Y - HKY_H/2.0;
        m_armXY_outOut.write();
        doShot  = false;
    }

    // 状態番号を更新する
    prevProc = nowProc;

    return RTC::RTC_OK;
}

/*
  RTC::ReturnCode_t AHAttack::onAborting(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t AHAttack::onError(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t AHAttack::onReset(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t AHAttack::onStateUpdate(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t AHAttack::onRateChanged(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/



extern "C"
{

    void AHAttackInit(RTC::Manager* manager)
    {
        coil::Properties profile(ahattack_spec);
        manager->registerFactory(profile,
                                 RTC::Create<AHAttack>,
                                 RTC::Delete<AHAttack>);
    }

};


