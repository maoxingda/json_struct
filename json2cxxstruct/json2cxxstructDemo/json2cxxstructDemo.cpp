// json2cxxstructDemo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <cassert>
#include <codecvt>
//#include "json2cxxstruct1.h"
#include "GeneratedFiles\json_json2cxxstruct1.h"

JSON_STRUCT(student)
{
	int id;
	wchar_t name[128];
	int qqs[2];

	student()
	{
		JSON_REGISTER_FIELD(id);
		JSON_REGISTER_FIELD(name);
		JSON_REGISTER_FIELD(qqs);
	}
};


int _tmain(int argc, _TCHAR* argv[])
{
	anchor a1;
	std::string anchor_json = std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(L"{\"retcode\":0,\"result\":{\"runtime\":0,\"state\":1,\"qs\":485448,\"residueTime\":176349,\"msg\":\"获得成功了\",\"isShow\":false,\"info\":{\"userId\":0,\"anchorId\":0,\"amount\":0,\"roomId\":0,\"userNickName\":\"齐齐达人\",\"anchorNickName\":\"齐齐达人\"}}}");
	//assert(a1.from_json(anchor_json));

	student s1;
	std::string stu_json = std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(L"{\"id\":1001,\"name\":\"毛兴达\",\"qqs\":[1,2,3]}");
	assert(s1.from_json(stu_json));

	return 0;
}

