#pragma once
#include "json_struct_base.h"


JSON_STRUCT(anchor_info)
{
	int userid;
	int anchorId;
	int amount;
	int roomId;
	wchar_t userNickName[256];
	wchar_t anchorNickName[256];

	anchor_info()
	{
		JSON_REGISTER_FIELD(userid);
		JSON_REGISTER_FIELD(anchorId);
		JSON_REGISTER_FIELD(amount);
		JSON_REGISTER_FIELD(roomId);
		JSON_REGISTER_FIELD(userNickName);
		JSON_REGISTER_FIELD(anchorNickName);
	}
};

JSON_STRUCT(res)
{
	int state;
	int qs;
	int residueTime;
	wchar_t msg[256];
	bool isShow;
	anchor_info info;

	res()
	{
		JSON_REGISTER_FIELD(state);
		JSON_REGISTER_FIELD(qs);
		JSON_REGISTER_FIELD(residueTime);
		JSON_REGISTER_FIELD(msg);
		JSON_REGISTER_FIELD(isShow);
		JSON_REGISTER_FIELD(info);
	}
};

JSON_STRUCT(anchor)
{
	int retcode;
	res result;

	anchor()
	{
		JSON_REGISTER_FIELD(retcode);
		JSON_REGISTER_FIELD(result);
	}
};