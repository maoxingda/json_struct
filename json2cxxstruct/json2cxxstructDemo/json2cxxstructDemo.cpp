// json2cxxstructDemo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <set>
#include <cassert>
#include <codecvt>
#include <typeinfo>
//#include "json2cxxstruct1.h"
#include "GeneratedFiles\json_json2cxxstruct1.h"


int _tmain(int argc, _TCHAR* argv[])
{
	int off = offsetof(student, off);
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> cnv;

	std::string stu_json = cnv.to_bytes(L"{\"id\":1001,\"qqs\":[1,2,3],\"name\":\"Ã«ÐË´ï\",\"birthday\":{\"year\":\"1990\",\"month\":\"02\",\"day\":\"16\"}}");

	student s1;

	assert(s1.from_json(stu_json));

	return 0;
}

