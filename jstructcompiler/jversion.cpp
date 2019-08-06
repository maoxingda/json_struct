#include "StdAfx.h"
#include "jversion.h"
#include "../util/jutil_common_path.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>


using namespace boost::property_tree;


jversion::jversion()
{
    ptree tree;

    read_xml(jutil_common_path().my_documents() + "\\Visual Studio 2010\\Addins\\version.xml", tree);

    version_ = tree.get<std::string>("compiler.version");
}
