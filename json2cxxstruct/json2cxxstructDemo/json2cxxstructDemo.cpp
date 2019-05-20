// json2cxxstructDemo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <cassert>
#include <codecvt>
#include "json2cxxstruct1.h"


int _tmain(int argc, _TCHAR* argv[])
{
	anchor a1;
	std::string anchor_json = std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(L"{\"retcode\":0,\"result\":{\"run-time\":0,\"state\":1,\"qs\":485448,\"residueTime\":176349,\"msg\":\"获得成功了\",\"isShow\":false,\"info\":{\"userId\":0,\"anchorId\":0,\"amount\":0,\"roomId\":0,\"userNickName\":\"齐齐达人\",\"anchorNickName\":\"齐齐达人\"}}}");
	assert(a1.from_json(anchor_json));
	return 0;
}

