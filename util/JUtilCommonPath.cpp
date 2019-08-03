#include "StdAfx.h"
#include <shlobj.h>
#include "JUtilCommonPath.h"


#pragma comment(lib, "shell32.lib")


std::string JUtilCommonPath::MyDocuments()
{
    SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, path);

    return path;
}