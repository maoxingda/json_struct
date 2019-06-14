#pragma once
#include "jmacro.h"
#include "jqualifier.h"
#include "jstruct_base.h"

#ifdef _DEBUG
#pragma comment(lib, "jstructd.lib")
#else
#pragma comment(lib, "jstruct.lib")
#endif // _DEBUG
