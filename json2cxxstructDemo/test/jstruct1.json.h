#include <jstruct.h>
#include <Windows.h>


struct struct_test_bool
{
    REQUIRED BASIC bool field_bool;
};

struct struct_test_number
{
    OPTIONAL BASIC int            field_int;
    REQUIRED BASIC __int64        field_int64;
    REQUIRED BASIC long           field_long;
    //REQUIRED BASIC unsigned short field_ushort;
    REQUIRED BASIC unsigned int   field_uint;
    REQUIRED BASIC unsigned long  field_ulong;
    REQUIRED BASIC float          field_float;
    REQUIRED BASIC double         field_double;
};

struct date
{
    REQUIRED BASIC wchar_t year[6];
    REQUIRED BASIC wchar_t month[4];
    REQUIRED BASIC wchar_t day[4];
};

struct student
{
    REQUIRED BASIC ALIAS(identifier) int     id;
    REQUIRED BASIC_ARRAY             int     qqs[2];
    REQUIRED BASIC                   wchar_t name[32];
    REQUIRED BASIC_ARRAY             wchar_t qqmail[3][32];
    REQUIRED CUSTOM                  date    birthday;
    REQUIRED CUSTOM_ARRAY            date    birthday_array[2];
};
