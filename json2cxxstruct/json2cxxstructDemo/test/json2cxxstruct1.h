#pragma once
#include <Windows.h>
#include "json_struct_base.h"


JSON_STRUCT(struct_test_bool)
{
	bool field_bool;

	JSON_STRUCT_DEF_CTOR(struct_test_bool);
};

JSON_STRUCT(struct_test_number)
{
	int				field_int;
	__int64			field_int64;
	long			field_long;
	unsigned short	field_ushort;
	unsigned int	field_uint;
	unsigned long	field_ulong;
	float			field_float;
	double			field_double;

	JSON_STRUCT_DEF_CTOR(struct_test_number);
};

JSON_STRUCT(date)
{
	wchar_t year[6];
	wchar_t month[4];
	wchar_t day[4];

	JSON_STRUCT_DEF_CTOR(date);
};

JSON_STRUCT(student)
{
	int		id;
	wchar_t name[32];
	JSON_STRUCT_FIELD(date birthday);

	JSON_STRUCT_DEF_CTOR(student);
};
