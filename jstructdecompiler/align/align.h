#pragma once
#include <string>


using namespace std;


class align
{
public:
    align(void);

    std::string align_field(const string& struct_name, const string& field_type);

    unsigned width_;
    unsigned max_width_;
    string   align_placeholder_;
};

