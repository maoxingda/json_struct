#pragma once
#include <jstruct.h>


/*bracket indicate optional
struct struct_name
{
    REQUIRED|OPTIONAL                     BASIC|BASIC_ARRAY|CUSTOM|CUSTOM_ARRAY [ALIAS(alias_name)]                   field_type field_name;
    REQUIRED|OPTIONAL                     [ALIAS(alias_name)]                   BASIC|BASIC_ARRAY|CUSTOM|CUSTOM_ARRAY field_type field_name;
    BASIC|BASIC_ARRAY|CUSTOM|CUSTOM_ARRAY REQUIRED|OPTIONAL                     [ALIAS(alias_name)]                   field_type field_name;
    BASIC|BASIC_ARRAY|CUSTOM|CUSTOM_ARRAY [ALIAS(alias_name)]                   REQUIRED|OPTIONAL                     field_type field_name;
    [ALIAS(alias_name)]                   REQUIRED|OPTIONAL                     BASIC|BASIC_ARRAY|CUSTOM|CUSTOM_ARRAY field_type field_name;
    [ALIAS(alias_name)]                   BASIC|BASIC_ARRAY|CUSTOM|CUSTOM_ARRAY REQUIRED|OPTIONAL                     field_type field_name;
    ...
};
*/

struct date
{
    REQUIRED BASIC wchar_t year[6];
    REQUIRED BASIC wchar_t month[4];
    REQUIRED BASIC wchar_t day[4];
};

struct student
{
    //REQUIRED BASIC ALIAS(id) int        identifier;
    REQUIRED BASIC           int        identifier;
    OPTIONAL BASIC           wchar_t    name[32];
    OPTIONAL BASIC_ARRAY     int        qq[5];
    OPTIONAL BASIC_ARRAY     wchar_t    email[5][32];
    REQUIRED CUSTOM          date       birthday;
};