// -*- C++ -*-
/*!
 * @file  PuckTracker.h
 * @brief ${rtcParam.description}
 * @date  $Date$
 *
 * $Id$
 */

#ifndef PUCKTRACKER_H
#define PUCKTRACKER_H

#include <rtm/Manager.h>
#include <rtm/DataFlowComponentBase.h>
#include <rtm/CorbaPort.h>
#include <rtm/DataInPort.h>
#include <rtm/DataOutPort.h>
#include <rtm/idl/BasicDataTypeSkel.h>
#include <rtm/idl/ExtendedDataTypesSkel.h>
#include <rtm/idl/InterfaceDataTypesSkel.h>

// Service implementation headers
// <rtc-template block="service_impl_h">

// </rtc-template>

// Service Consumer stub headers
// <rtc-template block="consumer_stub_h">
#include "AHCommonDataServiceStub.h"

// </rtc-template>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <sys/time.h>
#include <ctype.h>
#include <math.h>

// for opencv
#include <opencv/cv.h>
// #include <opencv/cvaux.h>
// #include <opencv/cvwimage.h>
#include <opencv/cxcore.h>
// #include <opencv/cxmisc.h>
#include <opencv/highgui.h>
// #include <ml.h>
#include <opencv2/legacy/legacy.hpp>


// for gpg-5300
#include <ifcml.h>


#define ImgWidth 640
#define ImgHeight 480
#define FRAMECNT	1

//#define S_MIN 0.7
//#define V_MIN 0.6
#define S_MIN 0.7
#define V_MIN 0.4
#define SIGMA 100.0
//#define SIGMA 50.0
#define N_PARTICLE 400
#define P_NOISE  30
#define V_NOISE  5


static inline void *_aligned_malloc(size_t size, size_t alignment)
{
    void *p;
    int ret = posix_memalign(&p, alignment, size);
    return (ret == 0) ? p : 0;
}


using namespace RTC;

/*!
 * @class PuckTracker
 * @brief ${rtcParam.description}
 *
 */
class PuckTracker
  : public RTC::DataFlowComponentBase
{
 public:
  /*!
   * @brief constructor
   * @param manager Maneger Object
   */
  PuckTracker(RTC::Manager* manager);

  /*!
   * @brief destructor
   */
  ~PuckTracker();

  // <rtc-template block="public_attribute">
  
  // </rtc-template>

  // <rtc-template block="public_operation">
  
  // </rtc-template>

  /***
   *
   * The initialize action (on CREATED->ALIVE transition)
   * formaer rtc_init_entry() 
   *
   * @return RTC::ReturnCode_t
   * 
   * 
   */
   virtual RTC::ReturnCode_t onInitialize();

  /***
   *
   * The finalize action (on ALIVE->END transition)
   * formaer rtc_exiting_entry()
   *
   * @return RTC::ReturnCode_t
   * 
   * 
   */
   virtual RTC::ReturnCode_t onFinalize();

  /***
   *
   * The startup action when ExecutionContext startup
   * former rtc_starting_entry()
   *
   * @param ec_id target ExecutionContext Id
   *
   * @return RTC::ReturnCode_t
   * 
   * 
   */
  // virtual RTC::ReturnCode_t onStartup(RTC::UniqueId ec_id);

  /***
   *
   * The shutdown action when ExecutionContext stop
   * former rtc_stopping_entry()
   *
   * @param ec_id target ExecutionContext Id
   *
   * @return RTC::ReturnCode_t
   * 
   * 
   */
  // virtual RTC::ReturnCode_t onShutdown(RTC::UniqueId ec_id);

  /***
   *
   * The activated action (Active state entry action)
   * former rtc_active_entry()
   *
   * @param ec_id target ExecutionContext Id
   *
   * @return RTC::ReturnCode_t
   * 
   * 
   */
   virtual RTC::ReturnCode_t onActivated(RTC::UniqueId ec_id);

  /***
   *
   * The deactivated action (Active state exit action)
   * former rtc_active_exit()
   *
   * @param ec_id target ExecutionContext Id
   *
   * @return RTC::ReturnCode_t
   * 
   * 
   */
   virtual RTC::ReturnCode_t onDeactivated(RTC::UniqueId ec_id);

  /***
   *
   * The execution action that is invoked periodically
   * former rtc_active_do()
   *
   * @param ec_id target ExecutionContext Id
   *
   * @return RTC::ReturnCode_t
   * 
   * 
   */
   virtual RTC::ReturnCode_t onExecute(RTC::UniqueId ec_id);

  /***
   *
   * The aborting action when main logic error occurred.
   * former rtc_aborting_entry()
   *
   * @param ec_id target ExecutionContext Id
   *
   * @return RTC::ReturnCode_t
   * 
   * 
   */
  // virtual RTC::ReturnCode_t onAborting(RTC::UniqueId ec_id);

  /***
   *
   * The error action in ERROR state
   * former rtc_error_do()
   *
   * @param ec_id target ExecutionContext Id
   *
   * @return RTC::ReturnCode_t
   * 
   * 
   */
  // virtual RTC::ReturnCode_t onError(RTC::UniqueId ec_id);

  /***
   *
   * The reset action that is invoked resetting
   * This is same but different the former rtc_init_entry()
   *
   * @param ec_id target ExecutionContext Id
   *
   * @return RTC::ReturnCode_t
   * 
   * 
   */
  // virtual RTC::ReturnCode_t onReset(RTC::UniqueId ec_id);
  
  /***
   *
   * The state update action that is invoked after onExecute() action
   * no corresponding operation exists in OpenRTm-aist-0.2.0
   *
   * @param ec_id target ExecutionContext Id
   *
   * @return RTC::ReturnCode_t
   * 
   * 
   */
  // virtual RTC::ReturnCode_t onStateUpdate(RTC::UniqueId ec_id);

  /***
   *
   * The action that is invoked when execution context's rate is changed
   * no corresponding operation exists in OpenRTm-aist-0.2.0
   *
   * @param ec_id target ExecutionContext Id
   *
   * @return RTC::ReturnCode_t
   * 
   * 
   */
  // virtual RTC::ReturnCode_t onRateChanged(RTC::UniqueId ec_id);


 protected:
  // <rtc-template block="protected_attribute">
  
  // </rtc-template>

  // <rtc-template block="protected_operation">
  
  // </rtc-template>

  // Configuration variable declaration
  // <rtc-template block="config_declare">
  /*!
   * 
   * - Name:  calibration_data
   * - DefaultValue: ./camera.xml
   */
  std::string m_calibration_data;
  /*!
   * 
   * - Name:  n_particle
   * - DefaultValue: 400
   */
  int m_n_particle;
  /*!
   * 
   * - Name:  p_noise
   * - DefaultValue: 30
   */
  int m_p_noise;
  /*!
   * 
   * - Name:  v_noise
   * - DefaultValue: 5
   */
  int m_v_noise;
  /*!
   * 
   * - Name:  sigma
   * - DefaultValue: 50.0
   */
  float m_sigma;
  /*!
   * 
   * - Name:  minH
   * - DefaultValue: 340.0
   */
  float m_minH;
  /*!
   * 
   * - Name:  maxH
   * - DefaultValue: 360.0
   */
  float m_maxH;
  /*!
   * 
   * - Name:  minS
   * - DefaultValue: 90.0
   */
  float m_minS;
  /*!
   * 
   * - Name:  maxS
   * - DefaultValue: 255.0
   */
  float m_maxS;
  /*!
   * 
   * - Name:  minV
   * - DefaultValue: 0.0
   */
  float m_minV;
  /*!
   * 
   * - Name:  maxV
   * - DefaultValue: 255.0
   */
  float m_maxV;
  /*!
   * 
   * - Name:  minR
   * - DefaultValue: 0.0
   */
  float m_minR;
  /*!
   * 
   * - Name:  maxR
   * - DefaultValue: 255.0
   */
  float m_maxR;
  /*!
   * 
   * - Name:  minG
   * - DefaultValue: 0.0
   */
  float m_minG;
  /*!
   * 
   * - Name:  maxG
   * - DefaultValue: 255.0
   */
  float m_maxG;
  /*!
   * 
   * - Name:  minB
   * - DefaultValue: 0.0
   */
  float m_minB;
  /*!
   * 
   * - Name:  maxB
   * - DefaultValue: 255.0
   */
  float m_maxB;
  /*!
   * 
   * - Name:  HSVorRGB
   * - DefaultValue: HSV
   */
  std::string m_HSVorRGB;
  // </rtc-template>

  // DataInPort declaration
  // <rtc-template block="inport_declare">
  
  // </rtc-template>


  // DataOutPort declaration
  // <rtc-template block="outport_declare">
  TimedDoubleSeq m_puckXY_out;
  /*!
   */
  OutPort<TimedDoubleSeq> m_puckXY_outOut;
  
  // </rtc-template>

  // CORBA Port declaration
  // <rtc-template block="corbaport_declare">
  /*!
   */
  RTC::CorbaPort m_AHServicePort;
  
  // </rtc-template>

  // Service declaration
  // <rtc-template block="service_declare">
  
  // </rtc-template>

  // Consumer declaration
  // <rtc-template block="consumer_declare">
  /*!
   */
  RTC::CorbaConsumer<AHCommon::AHCommonDataService> m_ahCommonDataService;
  
  // </rtc-template>

 private:
  // <rtc-template block="private_attribute">
  
  // </rtc-template>

  // <rtc-template block="private_operation">
  
  // </rtc-template>

};


extern "C"
{
  DLL_EXPORT void PuckTrackerInit(RTC::Manager* manager);
};

#endif // PUCKTRACKER_H
