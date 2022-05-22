#include "stdafx.h"

#include "Misc/Config.h"
#include "logging.h"
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <stdarg.h>

using namespace std;

// strftime format
#define LOGGER_TIME_FORMAT "%Y-%m-%d %H:%M:%S"

// printf format
#define LOGGER_MS_FORMAT ".%03d"

// convert current time to milliseconds since unix epoch
template <typename T>
static int get_ms(const std::chrono::time_point<T>& tp)
{
	using namespace std::chrono;

	auto dur = tp.time_since_epoch();
	auto s = std::chrono::duration_cast<std::chrono::seconds>(dur);
	std::chrono::duration<long, std::milli> rounded_ms = s;
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur);
	return static_cast<int>((ms - rounded_ms).count());
}

// format it in two parts: main part with date and time and part with milliseconds
static std::string format_time()
{
	auto tp = std::chrono::system_clock::now();
	std::time_t current_time = std::chrono::system_clock::to_time_t(tp);

	// this function use static global pointer. so it is not thread safe solution
	std::tm* time_info = std::localtime(&current_time);

	char buffer[128];

	int string_size = strftime(
	    buffer, sizeof(buffer),
	    LOGGER_TIME_FORMAT,
	    time_info);

	int ms = get_ms(tp);

	string_size += std::snprintf(
	    buffer + string_size, sizeof(buffer) - string_size,
	    LOGGER_MS_FORMAT, ms);

	return std::string(buffer, buffer + string_size);
}

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
	stream << "[" << format_time() << "] " << func << ":" << line << "\t- " << (msg ? msg : "NULL") << endl;

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
