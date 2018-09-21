#pragma once
#include "decls_public.gen.h"

/**
 * FnTable.h
 *
 * This defines all the getter methods for the plain-C thunk tables. For each available
 * interface, it defines FnTable::GetCVRxxxx_0xx(CVRxxxx_0xx *) - this method returns the
 * table for the named interface, and takes the instance to proxy to as an argument.
 *
 * Note this (the instance) is stored in a global variable, and there is not a seperate
 * dynamically-generated table per instance you pass in - the function will always return
 * the same pointer.
 *
 * See FnTable.cpp for more information
 *
 * Additionally, it defines OPENVR_FNTABLE_CALLTYPE - if you need to generate a function table
 * some other way, this correctly sets the call type.
 */

// Use stdcall on Windows, see openvr_capi.h
// Note that VC++ (and most other compilers) ignore calltype definitions on 64-bit, using fastcall instead. Not that it's
// relevant for 99% of this, but if you're getting mysterious bugs in 64-bit software don't think it's caused by this.
#if defined( _WIN32 )
#define OPENVR_FNTABLE_CALLTYPE __stdcall
#else
#define OPENVR_FNTABLE_CALLTYPE
#endif
