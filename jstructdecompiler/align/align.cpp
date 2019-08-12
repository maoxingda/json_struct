#include "align.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "../util/jutil_common_path.h"


using namespace boost::property_tree;


align::align(void)
    : reserve_arr_len_("2")
    , reserve_str_len_("512")
{
}

void align::load()
{
    ptree tree;

    read_xml(jutil_common_path().my_documents() + "\\Visual Studio 2010\\Addins\\fieldconf.xml", tree);

    reserve_arr_len_ = tree.get<std::string>("field.reserve_arr_len");
    reserve_str_len_ = tree.get<std::string>("field.reserve_str_len");
}

void align::save()
{
    ptree tree;

    tree.put("field.reserve_arr_len", reserve_arr_len_);
    tree.put("field.reserve_str_len", reserve_str_len_);

    write_xml(jutil_common_path().my_documents() + "\\Visual Studio 2010\\Addins\\fieldconf.xml", tree, std::locale(), xml_writer_settings<typename ptree::key_type>(' ', 4));
}
