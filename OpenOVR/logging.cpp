#include "stdafx.h"

#include "Misc/Config.h"
#include "logging.h"
#include <fstream>
#include <iostream>
#include <stdarg.h>

using namespace std;

#ifdef ANDROID
#include <android/log.h>
#else
static ofstream stream;
#endif

OC_NORETURN void oovr_abort_raw_va(const char* file, long line, const char* func, const char* msg, const char* title, va_list args);

void oovr_log_raw(const char* file, long line, const char* func, const char* msg)
{
#ifdef ANDROID
	__android_log_print(ANDROID_LOG_INFO, "OpenComposite", "%s:%d \t %s", func, line, msg);
#else
	if (!stream.is_open()) {
		stream.open("openovr_log");
	}

	// stream << file << ":" << line << ":" << func << "\t- " << msg << endl;
	stream << func << ":" << line << "\t- " << (msg ? msg : "NULL") << endl;

	// Write it to stdout
	// TODO on Windows, write it into the debug log
#ifndef _WIN32
	printf("[OC] %s:%ld \t %s\n", func, line, msg);
#endif

	// Do we need to close the stream or something? What about multiple threads?
#endif
}

void oovr_log_raw_format(const char* file, long line, const char* func, const char* msg, ...)
{
	va_list args;
	va_start(args, msg);

	char buff[2048];
	vsnprintf(buff, sizeof(buff), msg, args);

	oovr_log_raw(file, line, func, buff);

	va_end(args);
}

OC_NORETURN void oovr_abort_raw(const char* file, long line, const char* func, const char* msg, const char* title, ...)
{
	va_list args;
	va_start(args, title);
	oovr_abort_raw_va(file, line, func, msg, title, args);
	va_end(args); // Well we probably don't need this, but it should keep any tools happy
}

OC_NORETURN void oovr_abort_raw_va(const char* file, long line, const char* func, const char* msg, const char* title, va_list args)
{
	if (title == nullptr) {
		title = "OpenComposite Error - info in log";
		OOVR_LOG("Abort!");
	} else {
		OOVR_LOG(title);
	}

	char buff[2048];
	vsnprintf(buff, sizeof(buff), msg, args);
	buff[sizeof(buff) - 1] = 0;

	oovr_log_raw(file, line, func, buff);

	// Ensure everything gets written
#ifdef ANDROID
	__android_log_print(ANDROID_LOG_ERROR, "OpenComposite", "ERROR: %s:%d \t %s", func, line, buff);
#else
	stream << flush;
#endif

	OOVR_MESSAGE(buff, title);
	DebugBreak();
	exit(1);
}

void oovr_soft_abort_raw(const char* file, long line, const char* func, int* count, const char* msg, ...)
{
	// If this has been hit already, just ignore it - if we needed a crash that would've been done.
	if (*count > 0) {
		*count++;
		return;
	}

	va_list args;
	va_start(args, msg);

	// Otherwise, if we're in debug mode do a hard abort. Otherwise log and continue.
	// TODO fix this
	if (oovr_global_configuration.StopOnSoftAbort()) {
		oovr_abort_raw_va(file, line, func, msg, "OpenComposite Debug Error", args);
	}

	char buff[256];
	vsnprintf(buff, sizeof(buff), msg, args);
	buff[sizeof(buff) - 1] = 0;

	oovr_log_raw_format(file, line, func, "Soft Abort triggered (in non-debug mode, continuing - this will only print once): %s", buff);

	*count = 1;

	va_end(args);
}

void oovr_message_raw(const char* message, const char* title)
{
	// No need to log this, it will have already been done by the caller

#ifdef WIN32
	// Display a message box on Windows
	MessageBoxA(nullptr, message, title, MB_OK);
#else
	// Print to stderr on Linux
	cerr << "OOVR_MESSAGE: " << title << ": " << message << endl;
#endif
}

const float math_pi = 3.14159265358979323846f;
