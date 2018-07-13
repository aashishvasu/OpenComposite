// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "steamvr_abi.h"
#include "libovr_wrapper.h"

#include "Reimpl/CVRSystem015.h"
#include "Reimpl/CVRSystem017.h"
#include "Reimpl/CVRRenderModels005.h"
#include "Reimpl/CVRCompositor020.h"
#include "Reimpl/CVRCompositor022.h"
#include "Reimpl/CVROverlay017.h"

using namespace std;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
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
	MessageBoxA(NULL, buff, "OpenOVR Error", MB_OK);
	throw msg;
}

VR_INTERFACE void *VR_CALLTYPE VR_GetGenericInterface(const char * interfaceVersion, EVRInitError * error) {
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
		static CVR ## name ## _ ## version *val = nullptr; \
		if (!val) \
			val = new CVR ## name ## _ ## version(); \
		return val; \
	}

	INTERFACE(017, System);
	INTERFACE(015, System);
	INTERFACE(005, RenderModels);
	INTERFACE(020, Compositor);
	INTERFACE(022, Compositor);
	INTERFACE(017, Overlay);

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
	throw "stub";
}
