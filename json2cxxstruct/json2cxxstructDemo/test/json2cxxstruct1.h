#pragma once
#include <Windows.h>
#include "jstruct.h"


//JSTRUCT(struct_test_bool)
//{
//	REQUIRED bool field_bool;
//
//	struct_test_bool();
//	//JSON_STRUCT_DEF_CTOR(struct_test_bool);
//};

JSTRUCT(struct_test_number)
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

JSTRUCT(date)
{
	REQUIRED N wchar_t year[6];
	REQUIRED N wchar_t month[4];
	REQUIRED Y wchar_t day[4];

	date();
	//JSON_STRUCT_DEF_CTOR(date);
};

JSTRUCT(student)
{
	REQUIRED N int		id;
	REQUIRED N wchar_t	name[32];
	REQUIRED Y date		birthday;

	student();
	//JSON_STRUCT_DEF_CTOR(student);
};
