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


template <typename jstruct_type>
void jstruct_test(jstruct_type jstruct, std::string file_name)
{
	std::fstream json(file_name);

	if (json)
	{
		std::istreambuf_iterator<char> beg(json), end;

		std::string json_str(beg, end);

		jstruct_type obj;

		assert(obj.from_json(json_str));
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	struct_test_bool stb_bool;

	assert(stb_bool.from_json("{\"field_bool\":true}"));
	assert(stb_bool.from_json("{\"field_bool\":false}"));
	//////////////////////////////////////////////////////////////////////////
	jstruct_test(struct_test_number(),	"test/number.json");
	jstruct_test(student(),				"test/student.json");

	return true;
}
