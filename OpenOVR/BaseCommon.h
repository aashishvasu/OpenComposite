#pragma once
#include "OpenVR/interfaces/vrtypes.h"

class CVRCommon {
};

#ifdef BASE_IMPL

#define STUBBED() { \
	std::string str = "Hit stubbed file at " __FILE__ ":" + std::to_string(__LINE__) + " func " + std::string(__func__); \
	MessageBoxA(NULL, str.c_str(), "Stubbed func!", MB_OK); \
	throw "stub"; \
}

#endif

#define INTERFACE_FUNC(ret, name, ...) virtual ret name(__VA_ARGS__) override

#define CVR_GEN_IFACE() \
public: \
private: \
