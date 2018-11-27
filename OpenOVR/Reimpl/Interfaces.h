#pragma once
#include "BaseCommon.h"

// Note we return void* not CVRCommon* - that's due to vtable magic.
// Since our interfaces don't use CVRCommon as their first ancestor, this
// would return a reference to the CVRCommon part of the object.
void *CreateInterfaceByName(const char *name);

// Get information about a given interface
uint64_t GetInterfaceFlagsByName(const char *name, const char *flag, bool *success = nullptr);

// Use stdcall on Windows, see openvr_capi.h
// Note that VC++ (and most other compilers) ignore calltype definitions on 64-bit, using fastcall instead. Not that it's
// relevant for 99% of this, but if you're getting mysterious bugs in 64-bit software don't think it's caused by this.
#if defined( _WIN32 )
#define OPENVR_FNTABLE_CALLTYPE __stdcall
#else
#define OPENVR_FNTABLE_CALLTYPE
#endif
