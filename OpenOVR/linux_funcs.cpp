//
// Created by ZNix on 8/02/2021.
//

#include "stdafx.h"

#include "Misc/debugbreak.h"

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

#endif
