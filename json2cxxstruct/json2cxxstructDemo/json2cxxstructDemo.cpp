// json2cxxstructDemo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <set>
#include <cassert>
#include <codecvt>
#include <typeinfo>
#include "json2cxxstruct1.h"


JSON_STRUCT(test)
{
	wchar_t urls[4][128];

	test()
	{
		JSON_REGISTER_MAP_FIELD(urls, "urlsssss");
		JSON_STRUCT_FIELD_FILL_ZERO(test, urls);
	}
};


int _tmain(int argc, _TCHAR* argv[])
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> cnv;
	//////////////////////////////////////////////////////////////////////////
	std::string test_json = cnv.to_bytes(L"{\"urlsssss\":[\"url1\",\"url2\",\"url3\"]}");

	test t1;

	assert(t1.from_json(test_json));
	//////////////////////////////////////////////////////////////////////////
	std::string stu_json = cnv.to_bytes(L"{\"id\":1001,\"name\":\"Ã«ÐË´ï\",\"birthday1\":{\"year\":\"1990\"},\"birthday3\":[{\"year\":\"1990\"},{\"year\":\"1991\"},{\"year\":\"1992\"},{\"year\":\"1993\"}]}");

	student s1;

	assert(s1.from_json(stu_json));
	//////////////////////////////////////////////////////////////////////////
	return true;
}

