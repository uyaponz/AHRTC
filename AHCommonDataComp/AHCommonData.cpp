// -*- C++ -*-
/*!
 * @file  AHCommonData.cpp
 * @brief ${rtcParam.description}
 * @date $Date$
 *
 * $Id$
 */

#include "AHCommonData.h"

#include <iostream>
#include <string>

// Module specification
// <rtc-template block="module_spec">
static const char* ahcommondata_spec[] =
{
    "implementation_id", "AHCommonData",
    "type_name",         "AHCommonData",
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
    "conf.default.common_data", "../common/AHData.ini",
    // Widget
    "conf.__widget__.common_data", "text",
    // Constraints
    ""
};
// </rtc-template>

/*!
 * @brief constructor
 * @param manager Maneger Object
 */
AHCommonData::AHCommonData(RTC::Manager* manager)
    // <rtc-template block="initializer">
    : RTC::DataFlowComponentBase(manager),
      m_AHServicePort("AHService")

      // </rtc-template>
{
}

/*!
 * @brief destructor
 */
AHCommonData::~AHCommonData()
{
}



RTC::ReturnCode_t AHCommonData::onInitialize()
{
    // Registration: InPort/OutPort/Service
    // <rtc-template block="registration">
    // Set InPort buffers

    // Set OutPort buffer

    // Set service provider to Ports
    m_AHServicePort.registerProvider("AHCommonDataService", "AHCommon::AHCommonDataService", m_ahCommonDataService);

    // Set service consumers to Ports

    // Set CORBA Service Ports
    addPort(m_AHServicePort);

    // </rtc-template>

    // <rtc-template block="bind_config">
    // Bind variables and configuration variable
    bindParameter("common_data", m_common_data, "../common/AHData.ini");

    // </rtc-template>
    return RTC::RTC_OK;
}

/*
  RTC::ReturnCode_t AHCommonData::onFinalize()
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t AHCommonData::onStartup(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t AHCommonData::onShutdown(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/


RTC::ReturnCode_t AHCommonData::onActivated(RTC::UniqueId ec_id)
{
    std::cerr << "Reload hockey data." << std::endl;
    m_ahCommonDataService.reloadData(m_common_data);

    return RTC::RTC_OK;
}


RTC::ReturnCode_t AHCommonData::onDeactivated(RTC::UniqueId ec_id)
{
    return RTC::RTC_OK;
}


RTC::ReturnCode_t AHCommonData::onExecute(RTC::UniqueId ec_id)
{
    return RTC::RTC_OK;
}

/*
  RTC::ReturnCode_t AHCommonData::onAborting(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t AHCommonData::onError(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t AHCommonData::onReset(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t AHCommonData::onStateUpdate(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t AHCommonData::onRateChanged(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/



extern "C"
{

    void AHCommonDataInit(RTC::Manager* manager)
    {
        coil::Properties profile(ahcommondata_spec);
        manager->registerFactory(profile,
                                 RTC::Create<AHCommonData>,
                                 RTC::Delete<AHCommonData>);
    }

};


