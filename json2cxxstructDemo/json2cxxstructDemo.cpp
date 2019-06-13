// json2cxxstructDemo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <set>
#include <cassert>
#include <codecvt>
#include <typeinfo>
#include "json_jstruct1.json.h"
#include <fstream>
//#include <vld.h>


template <typename JSTRUCT_TYPE>
void jstruct_test(JSTRUCT_TYPE jstruct, std::string file_name)
{
	std::fstream json(file_name);

	if (json)
	{
		std::istreambuf_iterator<char> beg(json), end;

		std::string json_str(beg, end);

		JSTRUCT_TYPE obj;

		assert(obj.from_json(json_str));
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	struct_test_bool stb_true;

	assert(stb_true.from_json("{\"field_bool\":true}"));

	struct_test_bool stb_false;

	assert(stb_false.from_json("{\"field_bool\":false}"));
	//////////////////////////////////////////////////////////////////////////
	jstruct_test(struct_test_number(),	"test/number.json");
	jstruct_test(student(),				"test/student.json");

	return true;
}
