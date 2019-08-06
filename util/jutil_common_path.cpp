#include "StdAfx.h"
#include <shlobj.h>
#include "jutil_common_path.h"


#pragma comment(lib, "shell32.lib")


std::string jutil_common_path::my_documents()
{
    SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, path);

    return path;
}