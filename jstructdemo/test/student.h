// [assembly: Guid("3f54dc6b-a5e8-424f-8ace-f0cb67196ddd")] warning! do not delete this line
#pragma once
#include <jstruct.h>
#include "mjst/json_date.h"


struct student
{
    USER_T                   int        sex;
    //REQUIRED NUMBER_T ALIAS(id) int        identifier;
    REQUIRED NUMBER_T        int        identifier;
    OPTIONAL WCHAR_ARRAY_T   wchar_t    name[32];
    OPTIONAL NUMBER_ARRAY_T  int        qq[5];
    OPTIONAL WCHAR_TABLE_T   wchar_t    email[5][32];//[
    REQUIRED STRUCT_T        date       birthday;
};