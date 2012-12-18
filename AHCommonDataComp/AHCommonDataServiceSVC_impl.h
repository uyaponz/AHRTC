// -*-C++-*-
/*!
 * @file  AHCommonDataServiceSVC_impl.h
 * @brief Service implementation header of AHCommonDataService.idl
 *
 */

#include "AHCommonDataServiceSkel.h"

#include <string>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/optional.hpp>

#ifndef AHCOMMONDATASERVICESVC_IMPL_H
#define AHCOMMONDATASERVICESVC_IMPL_H

/*!
 * @class AHCommonDataServiceSVC_impl
 * Example class implementing IDL interface AHCommon::AHCommonDataService
 */
class AHCommonDataServiceSVC_impl
    : public virtual POA_AHCommon::AHCommonDataService,
      public virtual PortableServer::RefCountServantBase
{
private:
    boost::property_tree::ptree ahData_;

public:
    /*!
     * @brief standard constructor
     */
    AHCommonDataServiceSVC_impl();
    /*!
     * @brief destructor
     */
    virtual ~AHCommonDataServiceSVC_impl();

    // attributes and operations
    CORBA::Boolean getData(const char* dataname, CORBA::String_out data);
    void reloadData(const std::string filename);

};



#endif // AHCOMMONDATASERVICESVC_IMPL_H


