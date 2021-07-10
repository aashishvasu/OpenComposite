#pragma once
#include "OpenVR/interfaces/vrtypes.h"

class CVRCommon {
public:
	// Ensure the resources of the base get freed properly
	virtual ~CVRCommon() {}

	virtual void** _GetStatFuncList() = 0;
};

#if defined(BASE_IMPL) || defined(GENFILE)

#define STUBBED() { \
	std::string str = "Hit stubbed file at " __FILE__ ":" + std::to_string(__LINE__) + " func " + std::string(__func__); \
	OOVR_ABORT(str.c_str()); \
}

#endif

#if defined(GENFILE)
#define BASE_FLAG(...)
#define GEN_INTERFACE(if_name, version, ...)
#endif
