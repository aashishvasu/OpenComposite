// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifdef WIN32
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#endif

// TODO: reference additional headers your program requires here

// Disable the rgba and stpq vector components in GLM, as it makes debugging a huge pain
// Do this here to make sure it's observed everywhere. It shouldn't cause ABI problems, but
// it's still something we want to keep consistent.
#define GLM_FORCE_XYZW_ONLY

#define VR_API_EXPORT 1
// #include "OpenVR/openvr.h"
#include "custom_types.h"
#include "generated/interfaces/vrannotation.h"
#include "generated/interfaces/vrtypes.h"

#include "logging.h"

#include "Misc/xrutil.h"

#ifndef _WIN32
#include "linux_funcs.h"
#endif

#include <string>
#include <vector> // Super frequently used

// In the past, many files imported std ("using namespace std"). To make porting them over, and because
// it's convenient to use these shorthand, import them specifically.
using std::string;
using std::to_string;
using std::to_wstring;
using std::wstring;

// If we import math.h, then we'd like our maths constants
#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif

#ifdef WIN32
// This module's ID, from DLLMain
extern HMODULE openovr_module_id;
#else
// The handle returned from dlopen()
__attribute__((constructor)) void openovr_module_id();

// TODO Turtle1331 figure out how to write the shared library entry point (replacement for dllmain.cpp)
#endif
