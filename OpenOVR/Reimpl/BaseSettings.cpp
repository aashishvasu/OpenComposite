#include "stdafx.h"
#define BASE_IMPL
#include "BaseSettings.h"
#include "OpenVR/interfaces/IVRSettings_002.h"
#include <string>

#define STUBBED_BASIC() { \
	string str = "Hit stubbed file at " __FILE__ " func "  " line " + to_string(__LINE__); \
	MessageBoxA(NULL, str.c_str(), "Stubbed func!", MB_OK); \
	throw "stub"; \
}

#define STUBBED() { \
	string str = "Hit stubbed file at " __FILE__ " func "  " line " + to_string(__LINE__); \
	str += "via " + string(pchSection) + "." + string(pchSettingsKey); \
	MessageBoxA(NULL, str.c_str(), "Stubbed func!", MB_OK); \
	throw "stub"; \
}

#define UNSET_SETTING() { \
	string str = "Hit undefined setting at " __FILE__ " func "  " line " + to_string(__LINE__); \
	str += "via " + string(pchSection) + "." + string(pchSettingsKey); \
	MessageBoxA(NULL, str.c_str(), "Stubbed setting!", MB_OK); \
	throw "stub"; \
}

using namespace std;
namespace kk = vr::IVRSettings_002;

const char * BaseSettings::GetSettingsErrorNameFromEnum(EVRSettingsError eError) {
	switch (eError) {
	case VRSettingsError_None: return NULL;
	case VRSettingsError_IPCFailed: return "IPC Failed";
	case VRSettingsError_WriteFailed: return "Write Failed";
	case VRSettingsError_ReadFailed: return "Read Failed";
	case VRSettingsError_JsonParseFailed: return "JSON Parse Failed";
	case VRSettingsError_UnsetSettingHasNoDefault: return "IPC Failed";
	}
	OOVR_LOG(to_string(eError).c_str());
	STUBBED_BASIC();
}
bool  BaseSettings::Sync(bool bForce, EVRSettingsError * peError) {
	// We update everything immmediately, so AFAIK this shouldn't do anything?

	if (peError)
		*peError = VRSettingsError_None;

	return false;
}
void  BaseSettings::SetBool(const char * pchSection, const char * pchSettingsKey, bool bValue, EVRSettingsError * peError) {
	if (peError)
		*peError = VRSettingsError_None;

	STUBBED();
}
void  BaseSettings::SetInt32(const char * pchSection, const char * pchSettingsKey, int32_t nValue, EVRSettingsError * peError) {
	if (peError)
		*peError = VRSettingsError_None;

	STUBBED();
}
void  BaseSettings::SetFloat(const char * pchSection, const char * pchSettingsKey, float flValue, EVRSettingsError * peError) {
	if (peError)
		*peError = VRSettingsError_None;

	STUBBED();
}
void  BaseSettings::SetString(const char * pchSection, const char * pchSettingsKey, const char * pchValue, EVRSettingsError * peError) {
	if (peError)
		*peError = VRSettingsError_None;

	STUBBED();
}
bool  BaseSettings::GetBool(const char * pchSection, const char * pchSettingsKey, EVRSettingsError * peError) {
	string section = pchSection;
	string key = pchSettingsKey;

	if (peError)
		*peError = VRSettingsError_None;

	if (section == kk::k_pch_SteamVR_Section) {
		if (key == kk::k_pch_SteamVR_UsingSpeakers_Bool) {
			// True if the user is using external speakers (not attached to
			// their head), and the sound should thus be adjusted.
			// Note when set to true, expect k_pch_SteamVR_SpeakersForwardYawOffsetDegrees_Float
			return false; // TODO
		}
	}

	STUBBED();
}
int32_t  BaseSettings::GetInt32(const char * pchSection, const char * pchSettingsKey, EVRSettingsError * peError) {
	if (peError)
		*peError = VRSettingsError_None;

	STUBBED();
}
float  BaseSettings::GetFloat(const char * pchSection, const char * pchSettingsKey, EVRSettingsError * peError) {
	string section = pchSection;
	string key = pchSettingsKey;

	if (peError)
		*peError = VRSettingsError_None;

	if (section == kk::k_pch_SteamVR_Section) {
		if (key == kk::k_pch_SteamVR_SupersampleScale_Float) {
			return 1; // TODO
		}
	}

	UNSET_SETTING();
}
void  BaseSettings::GetString(const char * pchSection, const char * pchSettingsKey, VR_OUT_STRING() char * pchValue,
	uint32_t unValueLen, EVRSettingsError * peError) {

	if (peError)
		*peError = VRSettingsError_None;

	STUBBED();
}
void  BaseSettings::RemoveSection(const char * pchSection, EVRSettingsError * peError) {
	if (peError)
		*peError = VRSettingsError_None;

	STUBBED_BASIC();
}
void  BaseSettings::RemoveKeyInSection(const char * pchSection, const char * pchSettingsKey, EVRSettingsError * peError) {
	if (peError)
		*peError = VRSettingsError_None;

	STUBBED();
}
