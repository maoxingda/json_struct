#pragma once
#include <Windows.h>
#include "json_struct_base.h"


JSON_STRUCT(date)
{
	wchar_t year[6];
	/*
	wchar_t month[4];
	wchar_t day[4];*/

	JSON_STRUCT_DEF_CTOR(date);
};

JSON_STRUCT(student)
{
	int		id;
	JSON_STRUCT_FIELD(date birthday1);
	//int		qqs[2];
	wchar_t name[16];
	//JSON_STRUCT_FIELD(date birthday2);
	JSON_STRUCT_FIELD(date birthday3[3]);
	JSON_STRUCT_DEF_CTOR(student);
};