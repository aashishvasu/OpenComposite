// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

#include "plat.h"

#include "Reimpl/Interfaces.h"

// Needed for the system-wide usage of this DLL (when renamed to vrclient[_x64].dll)
#include "Reimpl/GVRClientCore.gen.h"

#include "steamvr_abi.h"
#include "Misc/debug_helper.h"
#include "Misc/Config.h"
#include <map>
#include <memory>
#include <functional>

// Specific to OCOVR
#include "Drivers/Backend.h"
#include "Drivers/DriverManager.h"
#include "DrvOpenXR.h"

using namespace std;

// Binary-compatible openvr_api.dll implementation
static bool running;
static bool running_ovr; // are we in an apptype which uses LibOVR?
static uint32_t current_init_token = 1;
static EVRApplicationType current_apptype;

static alternativeCoreFactory_t alternativeCoreFactory = nullptr;

OC_NORETURN void ERR(string msg) {
	char buff[4096];
	snprintf(buff, sizeof(buff), "OpenComposite DLLMain ERROR: %s", msg.c_str());
	OOVR_ABORT_T(buff, "OpenComposite Error");
}

class _InheritCVRLayout { virtual void _ignore() = 0; };
class CVRCorrectLayout : public _InheritCVRLayout, public CVRCommon {};

// If we don't set up our own deleter, then at least on linux it'll get the layout mixed up and call a random function
// Also for the basis of this typedef, see: https://stackoverflow.com/a/26276805
using correct_layout_unique = std::unique_ptr<CVRCorrectLayout, std::function<void(CVRCorrectLayout*)> >;

static map<string, correct_layout_unique> interfaces;

VR_INTERFACE void *VR_CALLTYPE VR_GetGenericInterface(const char * interfaceVersion, EVRInitError * error) {
	if (!running) {
		OOVR_LOGF("[INFO] VR_GetGenericInterface called while OOVR not running, setting error=NotInitialized, for interfaceVersion=%s", interfaceVersion);
		*error = VRInitError_Init_NotInitialized;
		return NULL;
	}

	// Unless we later change this otherwise, it was successful.
	// Oh, and who would pass in a null error pointer, since they're only called by `openvr_api.dll` which
	// defines all of them? HL:A of course!
	if (error)
		*error = VRInitError_None;

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

	if (interfaces.count(interfaceVersion)) {
		return interfaces[interfaceVersion].get();
	}

	// Hack for Half-Life: Alyx
	// This is an interface that is enabled or not by what seems to be a compile-time
	// switch inside SteamVR, disabled on the public build I was looking at.
	if (!strcmp("IXrProto_001", interfaceVersion)) {
		if (error)
			*error = VRInitError_Init_InterfaceNotFound;
		return NULL;
	}

	bool valid_apptypes_success;
	uint64_t valid_apptypes = GetInterfaceFlagsByName(interfaceVersion, "APPTYPE", &valid_apptypes_success);

	if (!valid_apptypes_success) {
		valid_apptypes = 1ull << VRApplication_Scene;
	}

	if ((valid_apptypes & (1ull << current_apptype)) == 0) {
		OOVR_LOGF("Invalid interface %s for apptype %d", interfaceVersion, current_apptype);
		OOVR_ABORT("Illegal interface for apptype - see log");
	}

	CVRCorrectLayout *impl = (CVRCorrectLayout*) CreateInterfaceByName(interfaceVersion);
	if (impl) {
		correct_layout_unique ptr(impl, [](CVRCorrectLayout* cl) {
			cl->Delete();
		});
		interfaces[interfaceVersion] = move(ptr);
		return impl;
	}

	OOVR_LOG(interfaceVersion);
	OOVR_MESSAGE(interfaceVersion, "Missing interface");
	ERR("unknown/unsupported interface " + string(interfaceVersion));
#undef INTERFACE
}

VR_INTERFACE uint32_t VR_CALLTYPE VR_GetInitToken() {
	return current_init_token;
}

VR_INTERFACE char * VR_GetStringForHmdError(int err) {
	OOVR_ABORT("Stub");
}

VR_INTERFACE const char *VR_CALLTYPE VR_GetVRInitErrorAsEnglishDescription(EVRInitError error) {
	switch (error) {
	case VRInitError_None:
		return "None";
	}
	OOVR_ABORT(("Init desc: Unknown value " + to_string(error)).c_str());
}

VR_INTERFACE const char *VR_CALLTYPE VR_GetVRInitErrorAsSymbol(EVRInitError error) {
	OOVR_ABORT("Stub");
}

VR_INTERFACE uint32_t VR_CALLTYPE VR_InitInternal(EVRInitError * peError, EVRApplicationType eApplicationType) {
	return VR_InitInternal2(peError, eApplicationType, NULL);
}

VR_INTERFACE uint32_t VR_CALLTYPE VR_InitInternal2(EVRInitError * peError, EVRApplicationType eApplicationType, const char * pStartupInfo) {
    *peError = VRInitError_None;

	if (eApplicationType == VRApplication_Bootstrapper) {
#ifdef _WIN32
		char szFileName[MAX_PATH];
		GetModuleFileNameA(NULL, szFileName, MAX_PATH);
		string filename = szFileName;

		filename.erase(0, filename.find_last_of('\\') + 1);

		if (filename == "vrstartup.exe") {
			// AFAIK vrstartup is the only program that uses this mode, and returning an
			// error makes it exit immediately.
			*peError = VRInitError_Unknown;
			return 0;
		}

		OOVR_ABORT("VRApplication_Bootstrapper currently only supported for vrstartup.exe");
#else
		LINUX_STUBBED();
#endif
	}

	if (eApplicationType == VRApplication_Utility) {
		return current_init_token;
	}

	if (eApplicationType != VRApplication_Scene)
		ERR("Cannot init VR: unsupported apptype " + to_string(eApplicationType));

	if (running)
		ERR("Cannot init VR: Already running!");

#ifndef OC_XR_PORT
	ovr::Setup();
#endif
	running_ovr = true;

success:
	current_apptype = eApplicationType;
	running = true;

	// TODO seperate this from the rest of dllmain
	BackendManager::Create(DrvOpenXR::CreateOpenXRBackend());
	// DriverManager::Instance().Register(DrvOculus::CreateOculusDriver());

	return current_init_token;
}

VR_INTERFACE bool VR_CALLTYPE VR_IsHmdPresent()
{
	// If we're already running, then Oculus's implementation fails with XR_ERROR_RUNTIME_FAILURE
	if (running)
		return true;

	// TODO properly test this

	XrSystemGetInfo systemInfo{};
	systemInfo.type = XR_TYPE_SYSTEM_GET_INFO;
	systemInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
	XrSystemId id;
	XrResult res = xrGetSystem(xr_instance, &systemInfo, &id);

	if (res == XR_SUCCESS)
		return true;

	if (res == XR_ERROR_FORM_FACTOR_UNAVAILABLE || res == XR_ERROR_FORM_FACTOR_UNSUPPORTED)
		return false;

	// Something else went wrong
	OOVR_ABORTF("Failed to probe for OpenXR systems: return status %d", res);
}

VR_INTERFACE bool VR_CALLTYPE VR_IsInterfaceVersionValid(const char * pchInterfaceVersion) {
	return true; // Kinda dodgy
}

VR_INTERFACE bool VR_CALLTYPE VR_IsRuntimeInstalled() {
	// TODO in future check that the Oculus Runtime is installed
	return true;
}

VR_INTERFACE const char *VR_CALLTYPE VR_RuntimePath() {
	OOVR_ABORT("Stub");
}

VR_INTERFACE void VR_CALLTYPE VR_ShutdownInternal() {
	// Reset interfaces
	// Do this first, while the OVR session is still available in case they
	//  need to use it for cleanup.
	interfaces.clear();

	// Shut down OpenXR
	BackendManager::Reset();

	running = false;
}

VR_INTERFACE void * VRClientCoreFactory(const char * pInterfaceName, int * pReturnCode) {
	if (alternativeCoreFactory) {
	use_alt:
		return alternativeCoreFactory(pInterfaceName, pReturnCode);
	}

	bool shouldUseOC = BaseClientCore::CheckAppEnabled();
	if (!shouldUseOC) {
		alternativeCoreFactory = PlatformGetAlternativeCoreFactory();

		// TODO use a more descriptive error message
		OOVR_FALSE_ABORT(alternativeCoreFactory);

		goto use_alt;
	}

	*pReturnCode = VRInitError_None;

	string name = pInterfaceName;

#define CLIENT_VER(ver) \
	if(IVRClientCore_ ## ver::IVRClientCore_Version == name) { \
		static CVRClientCore_ ## ver inst; \
		return &inst; \
	}

	CLIENT_VER(002);
	CLIENT_VER(003);

#undef CLIENT_VER

	OOVR_LOG(pInterfaceName);
	OOVR_MESSAGE(pInterfaceName, "Missing client interface");
	ERR("unknown/unsupported interface " + name);
}
