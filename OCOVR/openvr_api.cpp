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
static bool proton_hack = false; // Are we in the special proton status check mode?
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

VR_INTERFACE const char* VR_CALLTYPE VR_GetVRInitErrorAsEnglishDescription(EVRInitError error)
{
	switch (error) {
	case VRInitError_None:
		return "None";

		// Copied over from SteamVR, so these won't cause issues by not matching their SteamVR equivilents:
		// clang-format off
	case 100: return "Installation Not Found (100)";
	case 101: return "Installation Corrupt (101)";
	case 102: return "vrclient Shared Lib Not Found (102)";
	case 103: return "File Not Found (103)";
	case 104: return "Factory Function Not Found (104)";
	case 105: return "Interface Not Found (105)";
	case 106: return "Invalid Interface (106)";
	case 107: return "User Config Directory Invalid (107)";
	case 108: return "Hmd Not Found (108)";
	case 109: return "Not Initialized (109)";
	case 110: return "Installation path could not be located (110)";
	case 111: return "Config path could not be located (111)";
	case 112: return "Log path could not be located (112)";
	case 113: return "Unable to write path registry (113)";
	case 114: return "App info manager init failed (114)";
	case 115: return "Internal Retry (115)";
	case 116: return "User Canceled Init (116)";
	case 117: return "Another app was already launching (117)";
	case 118: return "Settings manager init failed (118)";
	case 119: return "VR system shutting down (119)";
	case 120: return "Too many tracked objects (120)";
	case 121: return "Not starting vrserver for background app (121)";
	case 122: return "The requested interface is incompatible with the compositor and the compositor is running (122)";
	case 123: return "This interface is not available to utility applications (123)";
	case 124: return "vrserver internal error (124)";
	case 125: return "Hmd DriverId is invalid (125)";
	case 126: return "Hmd Not Found Presence Failed (126)";
	case 127: return "VR Monitor Not Found (127)";
	case 128: return "VR Monitor startup failed (128)";
	case 129: return "Low Power Watchdog Not Supported (129)";
	case 130: return "Invalid Application Type (130)";
	case 131: return "Not available to watchdog apps (131)";
	case 132: return "Watchdog disabled in settings (132)";
	case 133: return "VR Dashboard Not Found (133)";
	case 134: return "VR Dashboard startup failed (134)";
	case 135: return "VR Home Not Found (135)";
	case 136: return "VR home startup failed (136)";
	case 137: return "Rebooting In Progress (137)";
	case 138: return "Firmware Update In Progress (138)";
	case 139: return "Firmware Recovery In Progress (139)";
	case 140: return "USB Service Busy (140)";
	case 141: return "VRInitError_Init_VRWebHelperStartupFailed";
	case 142: return "VRInitError_Init_TrackerManagerInitFailed";
	case 143: return "VRInitError_Init_AlreadyRunning";
	case 144: return "VRInitError_Init_FailedForVrMonitor";
	case 145: return "VRInitError_Init_PropertyManagerInitFailed";
	case 146: return "VRInitError_Init_WebServerFailed";
		// clang-format on

	default:
		return "VRInitError_Init_IllegalTypeTransition";
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

	// HACK for Proton: it calls InitInternal then (if successful) calls ShutdownInternal - just to see if the runtime is available
#ifndef _WIN32
    OOVR_FALSE_ABORT(!proton_hack);
	if (eApplicationType == VRApplication_Background) {
		proton_hack = true;
		// Don't mark us as running or anything, so VR_GetGenericInterface will still fail
		return 0;
	}
#endif

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
	if (proton_hack) {
		proton_hack = false;
		return;
	}

	OOVR_LOG("OpenComposite shutdown");

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
