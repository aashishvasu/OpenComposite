#pragma once

// TODO remove this
#include "../OpenOVR/logging.h"

#include <string>

#define STUBBED() { \
	std::string str = "Hit stubbed file at " __FILE__ ":" + std::to_string(__LINE__) + " func " + std::string(__func__); \
	OOVR_ABORT(str.c_str()); \
}
