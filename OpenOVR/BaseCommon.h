#pragma once
#include "OpenVR/interfaces/vrtypes.h"
#include "FnTable/FnTable.h"

class CVRCommon {
public:
	virtual void** _GetStatFuncList() = 0;
};

#if defined(BASE_IMPL) || defined(GENFILE)

#define STUBBED() { \
	std::string str = "Hit stubbed file at " __FILE__ ":" + std::to_string(__LINE__) + " func " + std::string(__func__); \
	OOVR_LOG(str.c_str()); \
	MessageBoxA(NULL, str.c_str(), "Stubbed func!", MB_OK); \
	throw "stub"; \
}

#endif

#if defined(CVR_IMPL)

#define CVR_GEN_IMPL(name) \
void** name::_GetStatFuncList() { \
	return FnTable::Get ## name(this); \
}; \

#endif

#if defined(GENFILE)
#define GEN_INTERFACE(if_name, version)
#endif

#define INTERFACE_FUNC(ret, name, ...) virtual ret name(__VA_ARGS__) override

#define CVR_GEN_IFACE() \
public: \
	virtual void** _GetStatFuncList() override; \
private: \
