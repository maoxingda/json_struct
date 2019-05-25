#pragma once
#include <Windows.h>
#include "jstruct.h"


JSTRUCT(struct_test_bool)
{
	REQUIRED BASIC bool field_bool;

	struct_test_bool();
};

JSTRUCT(struct_test_number)
{
	OPTIONAL BASIC int				field_int;
	REQUIRED BASIC __int64			field_int64;
	REQUIRED BASIC long				field_long;
	REQUIRED BASIC unsigned short	field_ushort;
	REQUIRED BASIC unsigned int		field_uint;
	REQUIRED BASIC unsigned long	field_ulong;
	REQUIRED BASIC float			field_float;
	REQUIRED BASIC double			field_double;

	struct_test_number();
};

JSTRUCT(date)
{
	REQUIRED BASIC wchar_t year[6];
	REQUIRED BASIC wchar_t month[4];
	REQUIRED BASIC wchar_t day[4];

	date();
};

JSTRUCT(student)
{
	REQUIRED BASIC			int			id;
	REQUIRED BASIC			wchar_t		name[32];
	REQUIRED CUSTOM			date		birthday;
	REQUIRED CUSTOM_ARRAY	date		birthday1[2];

	student();
};
