#pragma once
#include <string>

using namespace std;


enum type
{
    enum_none,

    enum_bool,

    enum_number,
    enum_number_array,

    enum_wchar_array,
    enum_wchar_table,

    enum_struct,
    enum_struct_array,
};

struct field_info
{
    size_t      type_   : 4;    // 15
    size_t      row_    : 16;   // 65535
    size_t      col_    : 16;   // 65535
    size_t      offset_ : 16;   // 65535

    void*       address_;       // save derived struct field address
    void*       address_size_;  // save derived struct array size field address

    string      type_name_;     // int float double etc.
    string      qualifier_;     // REQUIRED or OPTIONAL
    string      name_;          // c++ identifier
    string      alias_;         // use when name_ is not similar to json key name
};
