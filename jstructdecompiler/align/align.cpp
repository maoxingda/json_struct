#include "align.h"
//#include <boost/property_tree/ptree.hpp>
//#include <boost/property_tree/xml_parser.hpp>
//#include "../util/jutil_common_path.h"


//using namespace boost::property_tree;


align::align(void)
    : width_(0)
    , max_width_(8)
    , align_placeholder_("@maoxd")
{
    //ptree tree;

    //read_xml(jutil_common_path().my_documents() + "\\Visual Studio 2010\\Addins\\alignconf.xml", tree);

    //width_ = tree.get<unsigned>("align.width");
}

std::string align::align_field(const string& struct_name, const string& field_type)
{
    max_width_ < field_type.length() ? max_width_ = field_type.length() : 0;
    return '@' + struct_name + '@' + field_type + '@' + ' '/* + align_placeholder_*/;
}
