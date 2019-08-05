#include "StdAfx.h"
#include "JVersion.h"
#include "../util/JUtilCommonPath.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>


using namespace boost::property_tree;


JVersion::JVersion()
{
    ptree tree;

    read_xml(JUtilCommonPath().MyDocuments() + "\\Visual Studio 2010\\Addins\\version.xml", tree);

    version_ = tree.get<std::string>("compiler.version");
}
