#pragma once
#include <string>
#include <boost/xpressive/xpressive.hpp>


using namespace boost::xpressive;


class jalign
{
public:
    void align(std::string& to_be_align, std::string& ref, const sregex& re_to_be_align, const sregex& re_ref);
};

