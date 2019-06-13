#pragma once
#include "jmacro.h"
#include "jqualifier.h"
#include "jstruct_base.h"

#ifndef export_jstruct_lib
#ifdef _DEBUG
#pragma comment(lib, "json2cxxstructd.lib")
#else
#pragma comment(lib, "json2cxxstruct.lib")
#endif // _DEBUG
#endif