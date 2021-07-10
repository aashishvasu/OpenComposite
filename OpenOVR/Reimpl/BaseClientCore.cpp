#include "stdafx.h"
#define BASE_IMPL
#include "BaseClientCore.h"
#include "steamvr_abi.h"

#include <fstream>

#include <codecvt>

#include "Misc/json/json.h"

#include <ShlObj.h>

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
		OOVR_ABORTF("Failed to write applist json: %s", strerror(errno));
	}

	out << value;
}

enum Runtime {
	Default = 0,
	OpenComposite = 1,
	SteamVR = 2,
};

bool BaseClientCore::CheckAppEnabled() {
	wstring dlldir = GetDllDir();
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

	string apppath = GetAppPath();
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

template<typename str_t>
static void TrimPath(str_t &path) {
	if (path.back() == '\\' || path.back() == '/') {
		path.erase(path.end() - 1);
	}
}

template<typename str_t>
bool EndsWith(const str_t& a, const str_t& b) {
	if (b.size() > a.size()) return false;
	return std::equal(a.begin() + a.size() - b.size(), a.end(), b.begin());
}

// Derived from OpenVR
static wstring GetAppSettingsPath() {
	wstring path;
#if defined( WIN32 )
	WCHAR rwchPath[MAX_PATH];

	if (!SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, rwchPath))) {
		OOVR_ABORT("Failed to find the appdata directory");
	}

	path = rwchPath;
#elif defined( OSX )
#error "Unsupported platform"
#elif defined( LINUX )
#error "Unsupported platform"
#else
#error "Unsupported platform"
#endif

	TrimPath(path);

	return path;
}

static wstring GetOpenVRConfigPath() {
	wstring sConfigPath = GetAppSettingsPath();

#if defined( _WIN32 ) || defined( LINUX )
	sConfigPath += L"/openvr";
#elif defined ( OSX )
	sConfigPath += L"/.openvr";
#else
#error "Unsupported platform"
#endif
	return sConfigPath;
}

string BaseClientCore::GetAlternativeRuntimePath() {
	wstring regPath = GetOpenVRConfigPath();

#if defined( _WIN32 ) || defined( POSIX )
	regPath += L"/openvrpaths.vrpath";
#else
#error "Unsupported platform"
#endif

	Json::Value root;
	bool success = ReadJson(regPath, root);
	OOVR_FALSE_ABORT(success);

	const Json::Value runtimes = root["runtime"];
	if (!runtimes.isArray())
		OOVR_ABORT("Cannot start SteamVR: malformateed runtime path");

	for (unsigned int i = 0; i < runtimes.size(); i++) {
		string path = runtimes[i].asString();
		TrimPath(path);

		if (EndsWith(path, string("\\SteamVR")))
			return path;
	}

	OOVR_ABORT("Could not find a usable SteamVR intallation");
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

std::string BaseClientCore::GetAppPath() {
	char appExe[MAX_PATH];
	DWORD len = GetModuleFileNameA(nullptr, appExe, sizeof(appExe));
	return string(appExe, len);
}

std::wstring BaseClientCore::GetDllDir() {
	wchar_t buffer[MAX_PATH];
	DWORD len = GetModuleFileNameW(openovr_module_id, buffer, sizeof(buffer));
	wstring dllpath(buffer, len);
	return dllpath.substr(0, dllpath.find_last_of('\\') + 1);
}

void BaseClientCore::SetManifestPath(string filename) {
	wstring listname = GetDllDir() + L"applist.json";
	Json::Value root;
	ReadJson(listname, root);

	Json::Value &tag = root[GetAppPath()];

	// Don't need to update it
	if(tag["manifest"] == filename)
		return;

	tag["manifest"] = filename;
	WriteJson(listname, root);
}
