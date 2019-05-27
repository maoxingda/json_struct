// json2cxxstructDemo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <set>
#include <cassert>
#include <codecvt>
#include <typeinfo>
//#include "json2cxxstruct1.h"
#include "json_json2cxxstruct1.h"
//#include <vld.h>


int _tmain(int argc, _TCHAR* argv[])
{
	struct_test_bool stb_true;

	assert(stb_true.from_json("{\"field_bool\":true}"));

	struct_test_bool stb_false;

	assert(stb_false.from_json("{\"field_bool\":false}"));
	//////////////////////////////////////////////////////////////////////////
	struct_test_number stn;

	assert(stn.from_json("{\"field_int64\":4611686018427387904,\"field_long\":4611686018427387904,\"field_ushort\":456,\"field_uint\":2147483648,\"field_ulong\":2147483648,\"field_double\":3.1415926,\"field_float\":3.1415926}"));

	//////////////////////////////////////////////////////////////////////////
	std::wstring_convert < std::codecvt_utf8 < wchar_t >, wchar_t > cnv;

	student stu;

	assert(stu.from_json(cnv.to_bytes(L"{\"identifier\":1001,\"name\":\"Ã«ÐË´ï\",\"birthday\":{\"year\":\"1990\",\"month\":\"02\",\"day\":\"16\"},\"birthday_array\":[{\"year\":\"1990\",\"month\":\"02\",\"day\":\"16\"},{\"year\":\"1990\",\"month\":\"02\",\"day\":\"16\"}]}")));

	return true;
}
