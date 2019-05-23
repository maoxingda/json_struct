// json2cxxstructDemo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <set>
#include <cassert>
#include <codecvt>
#include <typeinfo>
#include "json2cxxstruct1.h"
//#include <vld.h>


int _tmain(int argc, _TCHAR* argv[])
{
	struct_test_bool stb_true;

	assert(stb_true.from_json("{\"field_bool\":true}"));

	struct_test_bool stb_false;

	assert(stb_false.from_json("{\"field_bool\":false}"));
	//////////////////////////////////////////////////////////////////////////
	struct_test_number stn;

	assert(stn.from_json("{\"field_int\":123,\"field_int64\":4611686018427387904,\"field_long\":4611686018427387904,\"field_ushort\":456,\"field_uint\":2147483648,\"field_ulong\":2147483648,\"field_double\":3.1415926,\"field_float\":3.1415926}"));

	char b = 128;
	return true;
}
