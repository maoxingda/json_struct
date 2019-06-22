#pragma once
#include <jstruct.h>


/*
#pragma once


// mandatory qualifier, specify one of them
#define OPTIONAL
#define REQUIRED

// mandatory qualifier, specify one of them
#define USER_T
#define BOOL_T

#define NUMBER_T
#define NUMBER_ARRAY_T

#define WCHAR_ARRAY_T
#define WCHAR_TABLE_T

#define STRUCT_T
#define STRUCT_ARRAY_T

// optional qualifier
#define ALIAS(name)
*/
struct date
{
    REQUIRED WCHAR_ARRAY_T wchar_t year[6];
    REQUIRED WCHAR_ARRAY_T wchar_t month[4];
    REQUIRED WCHAR_ARRAY_T wchar_t day[4];
};

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