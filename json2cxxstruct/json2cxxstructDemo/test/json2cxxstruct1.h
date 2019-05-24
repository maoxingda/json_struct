#pragma once
#include <Windows.h>
#include "json_struct_base.h"


//JSON_STRUCT(struct_test_bool)
//{
//	REQUIRED bool field_bool;
//
//	struct_test_bool();
//	//JSON_STRUCT_DEF_CTOR(struct_test_bool);
//};

JSON_STRUCT(struct_test_number)
{
	OPTIONAL int			field_int;
	//REQUIRED __int64		field_int64;
	//REQUIRED long			field_long;
	//REQUIRED unsigned short	field_ushort;
	//REQUIRED unsigned int	field_uint;
	//REQUIRED unsigned long	field_ulong;
	//REQUIRED float			field_float;
	//REQUIRED double			field_double;

	struct_test_number();
	//JSON_STRUCT_DEF_CTOR(struct_test_number);
};

JSON_STRUCT(date)
{
	REQUIRED wchar_t year[6];
	REQUIRED wchar_t month[4];
	REQUIRED wchar_t day[4];

	date();
	//JSON_STRUCT_DEF_CTOR(date);
};

JSON_STRUCT(student)
{
	REQUIRED int		id;
	REQUIRED wchar_t name[32];
	REQUIRED JSON_STRUCT_FIELD(date birthday);

	student();
	//JSON_STRUCT_DEF_CTOR(student);
};
