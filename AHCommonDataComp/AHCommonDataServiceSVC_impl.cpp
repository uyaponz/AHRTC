// -*-C++-*-
/*!
 * @file  AHCommonDataServiceSVC_impl.cpp
 * @brief Service implementation code of AHCommonDataService.idl
 *
 */

#include "AHCommonDataServiceSVC_impl.h"

#include <string>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/optional.hpp>


/*
 * Example implementational code for IDL interface AHCommon::AHCommonDataService
 */
AHCommonDataServiceSVC_impl::AHCommonDataServiceSVC_impl()
{
    // Please add extra constructor code here.
}


AHCommonDataServiceSVC_impl::~AHCommonDataServiceSVC_impl()
{
    // Please add extra destructor code here.
}


/*
 * Methods corresponding to IDL attributes and operations
 */
CORBA::Boolean AHCommonDataServiceSVC_impl::getData(
    const char* dataname,
    CORBA::String_out data)
{
    boost::optional<std::string> val = ahData_.get_optional<std::string>(dataname);
    if (val) {
        data = CORBA::string_dup(val.get().c_str());
        return true;
    }

    data = CORBA::string_dup("");
    return false;
}

void AHCommonDataServiceSVC_impl::reloadData(const std::string filename)
{
    boost::property_tree::read_ini(filename, ahData_);
}



// End of example implementational code
