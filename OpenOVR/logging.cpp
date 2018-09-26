#include "stdafx.h"
#include "logging.h"
#include <fstream>

using namespace std;

static ofstream stream;

void oovr_log_raw(const char *file, long line, const char *func, const char *msg) {
	if (!stream.is_open()) {
		stream.open("openovr_log");
	}

	//stream << file << ":" << line << ":" << func << "\t- " << msg << endl;
	stream << func << "\t- " << msg << endl;

	// Do we need to close the stream or something? What about multiple threads?
}

void oovr_abort_raw(const char * file, long line, const char * func, const char * msg, const char *title) {
	if (title == NULL) {
		title = "OpenComposite Error - info in log";
		OOVR_LOG("Abort!");
	}
	else {
		OOVR_LOG(title);
	}

	oovr_log_raw(file, line, func, msg);

	// Ensure everything gets written
	stream << flush;

	MessageBoxA(NULL, msg, title, MB_OK);
	exit(1);
}

const float math_pi = 3.14159265358979323846f;
