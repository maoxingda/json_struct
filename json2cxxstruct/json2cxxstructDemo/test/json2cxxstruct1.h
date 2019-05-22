#pragma once
#include <Windows.h>
#include "json_struct_base.h"


JSON_STRUCT(date)
{
	wchar_t year[6];
	wchar_t month[4];
	wchar_t day[4];
};

JSON_STRUCT(student)
{
	int		id;
	date	birthday1;
	int		qqs[2];
	wchar_t name[16];
	date	birthday2;
};