// -*- C++ -*-
/*!
 * @file  MalletTracker.cpp
 * @brief ${rtcParam.description}
 * @date $Date$
 *
 * $Id$
 */

#include "MalletTracker.h"
#include "ScanMallet.hpp"

#include <iostream>
#include <cstdio>
#include <cmath>
#include <limits>
#include <memory>

#include <stdlib.h>
#include <signal.h>

#include <opencv2/opencv.hpp>

#include "URGScanner.hpp"
#include "MyMath.hpp"
#include "MyFunc.hpp"

using namespace MyFunc;

const size_t g_sztMax = std::numeric_limits<size_t>::max();
const double g_dblMax = std::numeric_limits<double>::max();
const double g_dblNaN = std::numeric_limits<double>::quiet_NaN();

std::vector<URGController::ScanPoint_t> g_vecSP;


/* マレットの位置を検出する */
CvPoint2D64f getMalletXY(const std::vector<URGController::ScanPoint_t> &sp,
                         const URGController::URGRules &rules)
{
    CvPoint2D64f invalidPos = cvPoint2D64f(g_dblNaN, g_dblNaN);

    // 必要ないスキャン点を消す
    std::vector<URGController::ScanPoint_t> validSP; // 有効なスキャン点群
    {
        if (static_cast<int>(sp.size()) < rules.getSteps()) return invalidPos;

        // URG正面(+x方向) から -100.0[deg] まで
        for (size_t i=rules.getSteps() / 2;
             (rules.getResol() * (i - rules.getSteps()/2) >= -100.0) && (i <= sp.size());
             i--)
        {
            const double limitL = -hkyW/2.0 + invalidScanX;
            const double limitR =  hkyW/2.0 - invalidScanX;
            const double limitT =  hkyH/2.0 - invalidScanY;

            CvPoint2D64f v   = cvPoint2D64f(sp[i].x, sp[i].y);
            CvPoint2D64f pos = convU2H(v, cvPoint2D64f(urgOSX, urgOSY));
            if (limitL <= pos.x && pos.x <= limitR && pos.y <= limitT)
                validSP.push_back(sp[i]);
        }

        if (validSP.empty()) return invalidPos;
    }

    // URGから測定点までの距離が最小の点を探す
    size_t minN = g_sztMax; // 距離最小の添字(validSP)
    {
        double minL = g_dblMax;
        for (std::vector<URGController::ScanPoint_t>::iterator it=validSP.begin();
             it != validSP.end(); ++it)
        {
            double d = (*it).dist;
            if (minL > d) {
                minN = it - validSP.begin();
                minL = d;
            }
        }

        if (g_sztMax == minN) return invalidPos;
    }

    // URG -> マレット(仮) のベクトルとマレット座標(仮)を求める
    CvPoint2D64f vecU2M, posMallet; // URG->Malletのベクトル, マレットの台上座標
    {
        // 最近点からさらにマレット半径分先+15[mm]の位置までのベクトルを求める
        vecU2M = cvPoint2D64f(validSP[minN].x, validSP[minN].y);
        double lenU2M = sqrt( pow(vecU2M.x, 2) + pow(vecU2M.y, 2) );
        lenU2M /= (malletR/2.0-10.0);
        vecU2M = cvPoint2D64f(vecU2M.x + (vecU2M.x/lenU2M), vecU2M.y + (vecU2M.y/lenU2M));

        // マレット座標(仮)を求める
        posMallet = convU2H(vecU2M, cvPoint2D64f(urgOSX, urgOSY));
    }

    // 仮マレット座標を中心とした (malletR/2.0 + validSPMalletOffset) 以内
    // かつ、URG-仮マレット座標間に引いた線に垂直な線からURG側の点を取り出す
    std::vector<URGController::ScanPoint_t> newValidSP; // 分散の計算に使うスキャン点群
    {
        for (std::vector<URGController::ScanPoint_t>::iterator it=validSP.begin();
             it != validSP.end(); ++it)
        {
            CvPoint2D64f posScanPt = convU2H(cvPoint2D64f((*it).x, (*it).y),
                                             cvPoint2D64f(urgOSX,urgOSY));
            CvPoint2D64f vecM2Pt   = cvPoint2D64f(posScanPt.x - posMallet.x,
                                                  posScanPt.y - posMallet.y);
            CvPoint2D64f vecM2U_rot; {
                double x=-vecU2M.x, y=-vecU2M.y;
                double C=cos(M_PI/2.0), S=sin(M_PI/2.0);
                vecM2U_rot = cvPoint2D64f(x*C - y*S, x*S + y*C);
            }

            // 点までの距離が指定範囲以内か
            bool a = ( pow(malletR/2.0 + validSPMalletOffset, 2)
                       >= ( pow(vecM2Pt.x, 2) + pow(vecM2Pt.y, 2)) );
            // 水平線からURG側か
            bool b = ( 0 <= getCross(vecM2Pt, vecM2U_rot) );

            if (a && b) {
                URGController::ScanPoint_t pt = (*it);
                newValidSP.push_back(pt);
            }
        }

        if (newValidSP.empty()) return invalidPos;
    }

    // パーティクルを適当にばらまく
    std::vector<CvPoint2D64f> ptcl(particleN); // パーティクル的な何か
    {
        for (std::vector<CvPoint2D64f>::iterator it=ptcl.begin();
             it != ptcl.end(); ++it)
        {
//            // ただの正方形
//            double x = (static_cast<double>(rand()) / RAND_MAX) * malletR - (malletR/2.0);
//            double y = (static_cast<double>(rand()) / RAND_MAX) * malletR - (malletR/2.0);
//            (*it) = cvPoint2D64f(x + posMallet.x, y + posMallet.y);

            // マレット円形 (中心に行くほど密度が増す)
            double dist = (static_cast<double>(rand()) / RAND_MAX) * (malletR/2.0);
            double rad  = (static_cast<double>(rand()) / RAND_MAX) * (M_PI * 2.0);
            (*it) = cvPoint2D64f(dist*cos(rad) + posMallet.x, dist*sin(rad) + posMallet.y);
        }
    }

    // 各点とパーティクル座標間の距離の分散を計算する
    // (分散の最小値とその添字は実際には要らない？)
    std::vector<double> ptclVar(ptcl.size()); // パーティクル毎の分散
    double minVar    = g_dblMax; // 分散の最小値
    size_t minVarIdx = g_sztMax; // 分散が最小の添字(ptcl,ptclVar)
    {
        double spN = newValidSP.size();

        for (std::vector<CvPoint2D64f>::iterator itP=ptcl.begin();
             itP != ptcl.end(); ++itP)
        {
            // 平均と分散の計算は別関数化する

            // 平均
            double dMean = 0.0;
            for (std::vector<URGController::ScanPoint_t>::iterator itS=newValidSP.begin();
                 itS != newValidSP.end(); ++itS)
            {
                double px=(*itP).x, py=(*itP).y;
                double sx=convU2H_x((*itS).x, urgOSX), sy=convU2H_y((*itS).y, urgOSY);
                double dist = sqrt( pow(sx-px, 2) + pow(sy-py, 2) );
                dMean += dist;
            }
            dMean /= spN;

            // 分散
            double dVar = 0.0;
            for (std::vector<URGController::ScanPoint_t>::iterator itS=newValidSP.begin();
                 itS != newValidSP.end(); ++itS)
            {
                double px=(*itP).x, py=(*itP).y;
                double sx=convU2H_x((*itS).x, urgOSX), sy=convU2H_y((*itS).y, urgOSY);
                double dist = sqrt( pow(sx-px, 2) + pow(sy-py, 2) );
                dVar += pow(dMean - dist, 2);
            }
            dVar /= spN-1;

            // 計算した分散を記録する
            ptclVar[itP-ptcl.begin()] = dVar;

            // (一応)分散が最小のパーティクルを記憶する
            if (dVar < minVar) {
                minVar = dVar;
                minVarIdx = itP - ptcl.begin();
            }
        }

        // たぶん大丈夫だと思うけど...
        if (g_sztMax == minVarIdx) return invalidPos;
    }

    // 分散から求めた尤度を重みとする重み付き平均座標を求める
    std::vector<double> ptclLH(ptcl.size()); // パーティクルの尤度
    CvPoint2D64f posRealMallet; // 平均座標(=マレットの位置)
    {
        double sigma2 = ptclSigma2;

        for (std::vector<double>::iterator it=ptclVar.begin();
             it != ptclVar.end(); ++it)
        {
            // 尤度計算
            ptclLH[it-ptclVar.begin()] = (1 / sqrt(2*M_PI*sigma2)) * exp(-(*it) / (2*sigma2));
        }

        // 尤度による重み付き平均座標計算
        posRealMallet = cvPoint2D64f(0.0, 0.0);
        double sumLH = 0.0;
        for (std::vector<double>::iterator it=ptclLH.begin();
             it != ptclLH.end(); ++it)
        {
            posRealMallet.x += (*it) * ptcl[it-ptclLH.begin()].x;
            posRealMallet.y += (*it) * ptcl[it-ptclLH.begin()].y;
            sumLH += (*it);
        }
        posRealMallet.x /= sumLH;
        posRealMallet.y /= sumLH;
    }

    return posRealMallet;
}

// Module specification
// <rtc-template block="module_spec">
static const char* mallettracker_spec[] =
{
    "implementation_id", "MalletTracker",
    "type_name",         "MalletTracker",
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
    "conf.default.calibration_data", "./MalletCalibData.dat",
    "conf.default.n_particle", "200",
    "conf.default.sigma", "30.0",
    "conf.default.urg_pos_x", "-750.0",
    "conf.default.urg_pos_y", "0.0",
    "conf.default.urg_angle", "0.0",
    "conf.default.sp_scale_x", "1.055",
    "conf.default.sp_scale_y", "1.028",
    // Widget
    "conf.__widget__.calibration_data", "text",
    "conf.__widget__.n_particle", "text",
    "conf.__widget__.sigma", "text",
    "conf.__widget__.urg_pos_x", "text",
    "conf.__widget__.urg_pos_y", "text",
    "conf.__widget__.urg_angle", "text",
    "conf.__widget__.sp_scale_x", "text",
    "conf.__widget__.sp_scale_y", "text",
    // Constraints
    ""
};
// </rtc-template>

/*!
 * @brief constructor
 * @param manager Maneger Object
 */
MalletTracker::MalletTracker(RTC::Manager* manager)
    // <rtc-template block="initializer">
    : RTC::DataFlowComponentBase(manager),
      m_malletXY_outOut("MalletXY_out", m_malletXY_out),
      m_AHServicePort("AHService"),
      urg_("/dev/ttyACM0")

      // </rtc-template>
{
}

/*!
 * @brief destructor
 */
MalletTracker::~MalletTracker()
{
}



RTC::ReturnCode_t MalletTracker::onInitialize()
{
    // Registration: InPort/OutPort/Service
    // <rtc-template block="registration">
    // Set InPort buffers

    // Set OutPort buffer
    addOutPort("MalletXY_out", m_malletXY_outOut);

    // Set service provider to Ports

    // Set service consumers to Ports
    m_AHServicePort.registerConsumer("AHCommonDataService", "AHCommon::AHCommonDataService", m_ahCommonDataService);

    // Set CORBA Service Ports
    addPort(m_AHServicePort);

    // </rtc-template>

    // <rtc-template block="bind_config">
    // Bind variables and configuration variable
    bindParameter("calibration_data", m_calibration_data, "./MalletCalibData.dat");
    bindParameter("n_particle", m_n_particle, "200");
    bindParameter("sigma", m_sigma, "30.0");
    bindParameter("urg_pos_x", m_urg_pos_x, "-750.0");
    bindParameter("urg_pos_y", m_urg_pos_y, "0.0");
    bindParameter("urg_angle", m_urg_angle, "0.0");
    bindParameter("sp_scale_x", m_sp_scale_x, "1.055");
    bindParameter("sp_scale_y", m_sp_scale_y, "1.028");

    // </rtc-template>

    m_malletXY_out.data.length(2);
    m_malletXY_out.data[0] = m_malletXY_out.data[1] = 0.0;
    m_malletXY_outOut.write();

    srand(static_cast<unsigned>(time(NULL)));

    if (!urg_) {
        std::cerr << "Failed to open device (/dev/ttyACM0)." << std::endl;
        return RTC::RTC_ERROR;
    }
    rules_ = urg_.getURGRules();

    if (!loadCalibData("MalletCalibData.dat")) {
        std::cerr << "Failed to load calibration data." << std::endl;
        return RTC::RTC_ERROR;
    }
    Get_Calibration_Coefficient();

    return RTC::RTC_OK;
}


RTC::ReturnCode_t MalletTracker::onFinalize()
{
    return RTC::RTC_OK;
}

/*
  RTC::ReturnCode_t MalletTracker::onStartup(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t MalletTracker::onShutdown(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/


RTC::ReturnCode_t MalletTracker::onActivated(RTC::UniqueId ec_id)
{
    g_vecSP.clear();
    return RTC::RTC_OK;
}


RTC::ReturnCode_t MalletTracker::onDeactivated(RTC::UniqueId ec_id)
{
    return RTC::RTC_OK;
}


RTC::ReturnCode_t MalletTracker::onExecute(RTC::UniqueId ec_id)
{
    std::vector<URGController::ScanPoint_t> &sp = g_vecSP;

    /* scan mallet */
    do {
        if (0 >= urg_.getScanPoints(sp)) break;

        coil::TimeValue tm = coil::gettimeofday();

        flipScanY(sp);
        applyScanAngle(sp, urgRot);
        applyScanScale(sp, cvPoint2D64f(urgZX, urgZY));
        updateScanDist(sp);

        CvPoint2D64f posMallet = getMalletXY(sp, rules_);
        if (isnan(posMallet.x)) std::cerr << "invalid mallet position." << std::endl;
        else {
            Do_Calibration(posMallet.x, posMallet.y);
            std::cout << "mallet(x,y): (" << (posMallet.x) << ", " << (posMallet.y) << ")" << std::endl;
            m_malletXY_out.tm.sec  = tm.sec();
            m_malletXY_out.tm.nsec = tm.usec() * 1000;
            m_malletXY_out.data[0] = posMallet.x;
            m_malletXY_out.data[1] = posMallet.y;
            m_malletXY_outOut.write();
        }
    } while (false);

    return RTC::RTC_OK;
}

/*
  RTC::ReturnCode_t MalletTracker::onAborting(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t MalletTracker::onError(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t MalletTracker::onReset(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t MalletTracker::onStateUpdate(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t MalletTracker::onRateChanged(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/



extern "C"
{

    void MalletTrackerInit(RTC::Manager* manager)
    {
        coil::Properties profile(mallettracker_spec);
        manager->registerFactory(profile,
                                 RTC::Create<MalletTracker>,
                                 RTC::Delete<MalletTracker>);
    }

};


