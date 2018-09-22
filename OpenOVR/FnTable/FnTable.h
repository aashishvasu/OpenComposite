#pragma once

/**
 * FnTable.h
 *
 * Defines OPENVR_FNTABLE_CALLTYPE - if you need to generate a function table, this correctly sets the call type.
 */

// Use stdcall on Windows, see openvr_capi.h
// Note that VC++ (and most other compilers) ignore calltype definitions on 64-bit, using fastcall instead. Not that it's
// relevant for 99% of this, but if you're getting mysterious bugs in 64-bit software don't think it's caused by this.
#if defined( _WIN32 )
#define OPENVR_FNTABLE_CALLTYPE __stdcall
#else
#define OPENVR_FNTABLE_CALLTYPE
#endif
