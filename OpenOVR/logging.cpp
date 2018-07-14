#include "stdafx.h"
#include "logging.h"
#include <fstream>

using namespace std;

void oovr_log_raw(const char *file, long line, const char *func, const char *msg) {
	static ofstream stream("openovr_log");
	//stream << file << ":" << line << ":" << func << "\t- " << msg << endl;
	stream << func << "\t- " << msg << endl;

	// Do we need to close the stream or something? What about multiple threads?
}
