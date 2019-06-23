#pragma once


struct date
{
    REQUIRED WCHAR_ARRAY_T wchar_t year[6];
    REQUIRED WCHAR_ARRAY_T wchar_t month[4];
    REQUIRED WCHAR_ARRAY_T wchar_t day[4];
};
