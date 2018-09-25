// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>



// TODO: reference additional headers your program requires here

#define VR_API_EXPORT 1
//#include "OpenVR/openvr.h"
#include "OpenVR/interfaces/vrtypes.h"
#include "OpenVR/interfaces/vrannotation.h"
#include "custom_types.h"

#include "logging.h"

#include <string>

// This module's ID, from DLLMain
extern HMODULE openovr_module_id;
