#include "stdafx.h"
#define BASE_IMPL
#include "BaseClientCore.h"
#include "steamvr_abi.h"

#include <fstream>

#include <codecvt>

#include "Misc/json/json.h"

using namespace std;
using namespace vr;

static bool ReadJson(wstring path, Json::Value &result) {
	ifstream in(path, ios::binary);
	if (in) {
		std::stringstream contents;
		contents << in.rdbuf();
		contents >> result;
		return true;
	}
	else {
		result = Json::Value(Json::ValueType::objectValue);
		return false;
	}
}

static void WriteJson(wstring path, const Json::Value &value) {
	ofstream out(path, ios::binary);
	if (!out) {
		OOVR_ABORT("Failed to write applist json!");
	}

	out << value;
}

enum Runtime {
	Default = 0,
	OpenComposite = 1,
	SteamVR = 2,
};

bool BaseClientCore::CheckAppEnabled() {
	wchar_t buffer[MAX_PATH];
	DWORD len = GetModuleFileNameW(openovr_module_id, buffer, sizeof(buffer));
	wstring dllpath(buffer, len);
	wstring dlldir = dllpath.substr(0, dllpath.find_last_of('\\') + 1);
	wstring listname = dlldir + L"applist.json";
	wstring configname = dlldir + L"apps-config.json";

	Json::Value tag(Json::ValueType::objectValue);
	// Put information about this app in the tag
	{
		SYSTEMTIME st;
		GetSystemTime(&st);
		FILETIME ft; // Contains a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 (UTC).
		SystemTimeToFileTime(&st, &ft);
		ULARGE_INTEGER timestamp;
		timestamp.HighPart = ft.dwHighDateTime;
		timestamp.LowPart = ft.dwLowDateTime;

		// ms is now the number of milliseconds January 1, 1601 (UTC).
		uint64_t ms = timestamp.QuadPart / 10000;

		// Move to the UNIX epoch
		ms -= 11644473600000LL;

		tag["timestamp"] = Json::Value(ms);
	}

	char appExe[MAX_PATH];
	len = GetModuleFileNameA(0, appExe, sizeof(appExe));
	string apppath(appExe, len);

	Json::Value root;
	ReadJson(listname, root);

	root[apppath] = tag;

	WriteJson(listname, root);

	// Check what runtime the app should use, as set in the config menu
	Json::Value config;
	ReadJson(configname, config);
	Json::Value appconf = config[apppath];
	Json::Value global = config["***GlobalConfig"];

	// Keep track of what the selected runtime is
	Runtime selected = Runtime::Default;

	// If a runtime has been set for this program in the GUI, use that
	if (appconf.isMember("runtime")) {
		selected = (Runtime)appconf["runtime"].asInt();
	}

	// If not, then check if a default runtime has been specified
	if (selected == Runtime::Default && global.isMember("default_runtime")) {
		selected = (Runtime)global["default_runtime"].asInt();
	}

	// Finally, default to using OpenComposite
	if (selected == Runtime::Default) {
		selected = Runtime::OpenComposite;
	}

	// TODO check a list of enabled apps, and return whether we should use OpenComposite based on that
	return selected == Runtime::OpenComposite;
}

EVRInitError BaseClientCore::Init(vr::EVRApplicationType eApplicationType, const char * pStartupInfo) {
	EVRInitError err;
	VR_InitInternal2(&err, eApplicationType, pStartupInfo);
	return err;
}

void BaseClientCore::Cleanup() {
	// Note that this object is not affected by the shutdown, as it is handled seperately
	//  from all the other interface objects and is only destroyed when the DLL is unloaded.
	VR_ShutdownInternal();
}

EVRInitError BaseClientCore::IsInterfaceVersionValid(const char * pchInterfaceVersion) {
	return VR_IsInterfaceVersionValid(pchInterfaceVersion) ? VRInitError_None : VRInitError_Init_InvalidInterface;
}

void * BaseClientCore::GetGenericInterface(const char * pchNameAndVersion, EVRInitError * peError) {
	return VR_GetGenericInterface(pchNameAndVersion, peError);
}

bool BaseClientCore::BIsHmdPresent() {
	return VR_IsHmdPresent();
}

const char * BaseClientCore::GetEnglishStringForHmdError(vr::EVRInitError eError) {
	return VR_GetVRInitErrorAsEnglishDescription(eError);
}

const char * BaseClientCore::GetIDForVRInitError(vr::EVRInitError eError) {
	return VR_GetVRInitErrorAsSymbol(eError);
}
