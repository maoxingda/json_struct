#pragma once
#include <Windows.h>
#include "json_struct_base.h"


JSON_STRUCT(anchor_info)
{
	int				userid;					//用户Id
	int				anchorId;
	int				amount;
	int				roomId;
	wchar_t			userNickName[256];
	wchar_t			anchorNickName[256];
};

JSON_STRUCT(res)
{
	int				runtime;
	int				state;
	int				qs;
	int				residueTime;
	wchar_t			m_wzMsg[256];
	bool			isShow;
	anchor_info		info;
};

JSON_STRUCT(anchor)
{
	BOOL			retcode;
	res				result;
};