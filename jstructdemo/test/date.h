// [assembly: Guid("3f54dc6b-a5e8-424f-8ace-f0cb67196ddd")] warning! do not delete this line
#pragma once
#include <jstruct.h>


struct date
{
    REQUIRED WCHAR_ARRAY_T wchar_t year[6];
    REQUIRED WCHAR_ARRAY_T wchar_t month[4];
    REQUIRED WCHAR_ARRAY_T wchar_t day[4];
};
