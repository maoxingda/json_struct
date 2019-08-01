#include "StdAfx.h"
#include <shlobj.h>
#include "UtilCommonPath.h"


#pragma comment(lib, "shell32.lib")

std::string UtilCommonPath::MyDocuments()
{
    SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, path);

    return path;
}