#pragma once
#include <Windows.h>
#include "json_struct_base.h"


JSON_STRUCT(struct_test_bool)
{
	bool field_bool;

	JSON_STRUCT_DEF_CTOR(struct_test_bool);
};

//note data range
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
