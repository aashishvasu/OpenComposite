#include "stdafx.h"

#include "logging.h"
#include <fstream>

#ifndef WIN32
#include <iostream>
#endif

using namespace std;

static ofstream stream;

void oovr_log_raw(const char *file, long line, const char *func, const char *msg) {
	if (!stream.is_open()) {
		stream.open("openovr_log");
	}

	//stream << file << ":" << line << ":" << func << "\t- " << msg << endl;
	stream << func << "\t- " << (msg ? msg : "NULL") << endl;

	// Do we need to close the stream or something? What about multiple threads?
}

void oovr_log_raw_format(const char *file, long line, const char *func, const char *msg, ...) {
	va_list args;
	va_start(args, msg);

	char buff[2048];
	vsnprintf(buff, sizeof(buff), msg, args);

	oovr_log_raw(file, line, func, buff);

	va_end(args);
}

void oovr_abort_raw(const char * file, long line, const char * func, const char * msg, const char *title, ...) {
	if (title == nullptr) {
		title = "OpenComposite Error - info in log";
		OOVR_LOG("Abort!");
	}
	else {
		OOVR_LOG(title);
	}

	va_list args;
	va_start(args, title);
	char buff[2048];
	vsnprintf(buff, sizeof(buff), msg, args);
	buff[sizeof(buff) - 1] = 0;

	oovr_log_raw(file, line, func, buff);

	// Ensure everything gets written
	stream << flush;

	OOVR_MESSAGE(buff, title);
	exit(1);
}

void oovr_message_raw(const char *message, const char *title) {
	// Save to log file
	oovr_log_raw_format("logging.h", 14, "OOVR_MESSAGE", "%s: %s", title, message);

	#ifdef WIN32
		// Display a message box on Windows
		MessageBoxA(NULL, message, title, MB_OK);
	#else
		// Print to stderr on Linux
		cerr << "OOVR_MESSAGE: " << title << ": " << message;
	#endif
}

const float math_pi = 3.14159265358979323846f;
