//
// Created by ZNix on 8/02/2021.
//

#include "stdafx.h"

#include "Misc/debugbreak.h"
#include "resources.h"

#include <stdlib.h>

#ifndef _WIN32

void ZeroMemory(void* data, size_t len)
{
	memset(data, 0, len);
}

void strncpy_s(char* dest, size_t dest_size, char const* src, size_t max)
{
	// Probably not quite right - this is basically the same as strcpy_s - but it should do
	OOVR_FALSE_ABORT(strlen(src) < max);
	OOVR_FALSE_ABORT(strlen(src) < dest_size);
	strncpy(dest, src, dest_size);
}

void strcpy_s(char* dest, size_t dest_size, char const* src)
{
	OOVR_FALSE_ABORT(strlen(src) < dest_size);
	strcpy(dest, src);
}

void DebugBreak()
{
	// This will cause a crash if we're not being debugged, so require an environment variable to be set and non-empty.
	const char* debug_break_env = getenv("OPENCOMPOSITE_DEBUG_BREAK");
	if (debug_break_env && debug_break_env[0] != 0)
		debug_break();
}

// Our hacky resource lookup implementation - a symbol is defined for the start and end of each file. We then have a case
// that grabs the correct symbols, so it's roughly-ish usable in place of the Windows resource system.
#define DEF_RESOURCE(name)                   \
	extern "C" const char resource_##name[]; \
	extern "C" const char resource_##name##_end[];

RES_LIST_LINUX(DEF_RESOURCE)
#undef DEF_RESOURCE

void FindResourceLinux(int id, const char** start, const char** end)
{
	switch (id) {
#define RESOURCE_CASE(name)           \
	case name:                        \
		*start = resource_##name;     \
		*end = resource_##name##_end; \
		break;

		RES_LIST_LINUX(RESOURCE_CASE);
#undef RESOURCE_CASE
	default:
		OOVR_ABORTF("Invalid resource ID %d", id);
	}
}

#endif
