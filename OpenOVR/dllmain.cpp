// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

#include "Reimpl/GVRSystem.gen.h"
#include "Reimpl/CVRRenderModels005.h"
#include "Reimpl/GVRCompositor.gen.h"
#include "Reimpl/GVROverlay.gen.h"
#include "Reimpl/CVRSettings002.h"
#include "Reimpl/GVRChaperone.gen.h"
#include "Reimpl/GVRChaperoneSetup.gen.h"
#include "Reimpl/CVRScreenshots001.h"

#include "steamvr_abi.h"
#include "libovr_wrapper.h"
#include "Misc/debug_helper.h"

#define INTERFACE_LIST(INTERFACE) \
INTERFACE(017, System); \
INTERFACE(016, System); \
INTERFACE(015, System); \
INTERFACE(005, RenderModels); \
INTERFACE(020, Compositor); \
INTERFACE(022, Compositor); \
INTERFACE(017, Overlay); \
INTERFACE(016, Overlay); \
INTERFACE(002, Settings); \
INTERFACE(003, Chaperone); \
INTERFACE(005, ChaperoneSetup); \
INTERFACE(001, Screenshots); \
;

using namespace std;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
#if defined(_DEBUG)
		DbgSetModule(hModule);
#endif
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

// Binary-compatible openvr_api.dll implementation
static bool running;
static uint32_t current_init_token = 1;

void ERR(string msg) {
	char buff[4096];
	snprintf(buff, sizeof(buff), "OpenOVR DLLMain ERROR: %s", msg.c_str());
	OOVR_LOG(buff);
	MessageBoxA(NULL, buff, "OpenOVR Error", MB_OK);
	throw msg;
}

#define INTERFACE(version, name) static CVR ## name ## _ ## version *instance_ ## name ## _ ## version = nullptr;
INTERFACE_LIST(INTERFACE);
#undef INTERFACE

class _InheritCVRLayout { virtual void _ignore() = 0; };
class CVRCorrectLayout : public _InheritCVRLayout, public CVRCommon {};

VR_INTERFACE void *VR_CALLTYPE VR_GetGenericInterface(const char * interfaceVersion, EVRInitError * error) {
	if (!running) {
		OOVR_LOG("[INFO] VR_GetGenericInterface called while OOVR not running, setting error=NotInitialized");
		*error = VRInitError_Init_NotInitialized;
		return NULL;
	}

	// First check if they're getting the 'FnTable' version of this interface.
	// This is a table of methods, but critically they *don't* take a 'this' pointer,
	//  so we can't cheat and return the vtable.
	const char *fnTableStr = "FnTable:";
	if (!strncmp(fnTableStr, interfaceVersion, strlen(fnTableStr))) {
		const char *baseInterface = interfaceVersion + strlen(fnTableStr);

		// Get the C++ interface
		// Note we can't directly cast to CVRCommon, as we'll then be referring to the OpenVR interface
		// vtable - look up how vtables work with multiple inheritance if you're confused about this.
		CVRCorrectLayout *interfaceClass = (CVRCorrectLayout*) VR_GetGenericInterface(baseInterface, error);

		// If the interface is NULL, then error will have been set and we can return null too.
		if (!interfaceClass) {
			return NULL;
		}

		return interfaceClass->_GetStatFuncList();
	}

	// Notes on the interface macro:
	// If interfaceVersion is the same as IVRname_Version, it returns a copy
	// of CVRname, which is cached in the variable CVRImplAccess.vr_name
	// Ex: if name is System:
	//   target interface name is IVRSystem_Version
	//   Instance of object CVRSystem
	//   cached in CVRImplAccess.vr_System
#define INTERFACE(version, name) \
	/* Just a reminder 0==false and strcmp returns 0 if the strings match */ \
	if (!strcmp(vr::IVR ## name ## _ ## version :: IVR ## name ## _Version, interfaceVersion)) { \
		auto &val = instance_ ## name ## _ ## version; \
		if (!val) \
			val = new CVR ## name ## _ ## version(); \
		return val; \
	}

	INTERFACE_LIST(INTERFACE);

#undef INTERFACE

	OOVR_LOG(interfaceVersion);
	MessageBoxA(NULL, interfaceVersion, "Missing interface", MB_OK);
	ERR("unknown/unsupported interface " + string(interfaceVersion));
#undef INTERFACE
}

VR_INTERFACE uint32_t VR_CALLTYPE VR_GetInitToken() {
	return current_init_token;
}

VR_INTERFACE char * VR_GetStringForHmdError(int err) {
	throw "stub";
}

VR_INTERFACE const char *VR_CALLTYPE VR_GetVRInitErrorAsEnglishDescription(EVRInitError error) {
	throw "stub";
}

VR_INTERFACE const char *VR_CALLTYPE VR_GetVRInitErrorAsSymbol(EVRInitError error) {
	throw "stub";
}

VR_INTERFACE uint32_t VR_CALLTYPE VR_InitInternal(EVRInitError * peError, EVRApplicationType eApplicationType) {
	return VR_InitInternal2(peError, eApplicationType, NULL);
}

VR_INTERFACE uint32_t VR_CALLTYPE VR_InitInternal2(EVRInitError * peError, EVRApplicationType eApplicationType, const char * pStartupInfo) {
	// TODO use peError

	if (eApplicationType != VRApplication_Scene)
		ERR("Cannot init VR: unsuported apptype " + to_string(eApplicationType));

	if (running)
		ERR("Cannot init VR: Already running!");

	ovr::Setup();
	running = true;

	*peError = VRInitError_None;

	return current_init_token;
}

VR_INTERFACE bool VR_CALLTYPE VR_IsHmdPresent() {
	return ovr::IsAvailable();
}

VR_INTERFACE bool VR_CALLTYPE VR_IsInterfaceVersionValid(const char * pchInterfaceVersion) {
	return true; // Kinda dodgy
}

VR_INTERFACE bool VR_CALLTYPE VR_IsRuntimeInstalled() {
	// TODO in future check that the Oculus Runtime is installed
	return true;
}

VR_INTERFACE const char *VR_CALLTYPE VR_RuntimePath() {
	throw "stub";
}

VR_INTERFACE void VR_CALLTYPE VR_ShutdownInternal() {
	// Reset interfaces
	// Do this first, while the OVR session is still available in case they
	//  need to use it for cleanup.
#define INTERFACE(version, name) { \
		auto &val = instance_ ## name ## _ ## version; \
		if (val) delete val; \
		val = nullptr; \
	}
	INTERFACE_LIST(INTERFACE);
#undef INTERFACE

	// Shut down LibOVR
	ovr::Shutdown();
	running = false;
}
