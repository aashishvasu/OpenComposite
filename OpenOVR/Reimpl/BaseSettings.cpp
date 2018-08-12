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
	STUBBED_BASIC();
}
bool  BaseSettings::Sync(bool bForce, EVRSettingsError * peError) {
	STUBBED_BASIC();
}
void  BaseSettings::SetBool(const char * pchSection, const char * pchSettingsKey, bool bValue, EVRSettingsError * peError) {
	STUBBED();
}
void  BaseSettings::SetInt32(const char * pchSection, const char * pchSettingsKey, int32_t nValue, EVRSettingsError * peError) {
	STUBBED();
}
void  BaseSettings::SetFloat(const char * pchSection, const char * pchSettingsKey, float flValue, EVRSettingsError * peError) {
	STUBBED();
}
void  BaseSettings::SetString(const char * pchSection, const char * pchSettingsKey, const char * pchValue, EVRSettingsError * peError) {
	STUBBED();
}
bool  BaseSettings::GetBool(const char * pchSection, const char * pchSettingsKey, EVRSettingsError * peError) {
	STUBBED();
}
int32_t  BaseSettings::GetInt32(const char * pchSection, const char * pchSettingsKey, EVRSettingsError * peError) {
	STUBBED();
}
float  BaseSettings::GetFloat(const char * pchSection, const char * pchSettingsKey, EVRSettingsError * peError) {
	string section = pchSection;
	string key = pchSettingsKey;

	if (section == kk::k_pch_SteamVR_Section) {
		if (key == kk::k_pch_SteamVR_SupersampleScale_Float) {
			return 1; // TODO
		}
	}

	UNSET_SETTING();
}
void  BaseSettings::GetString(const char * pchSection, const char * pchSettingsKey, VR_OUT_STRING() char * pchValue,
	uint32_t unValueLen, EVRSettingsError * peError) {
	STUBBED();
}
void  BaseSettings::RemoveSection(const char * pchSection, EVRSettingsError * peError) {
	STUBBED_BASIC();
}
void  BaseSettings::RemoveKeyInSection(const char * pchSection, const char * pchSettingsKey, EVRSettingsError * peError) {
	STUBBED();
}
