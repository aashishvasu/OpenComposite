#include "stdafx.h"
#define BASE_IMPL
#include "BaseSettings.h"
#include "OpenVR/interfaces/IVRSettings_001.h"
#include "OpenVR/interfaces/IVRSettings_002.h"
#include "Misc/Config.h"
#include "BaseSystem.h"
#include <string>

#ifndef OC_XR_PORT
// Required for the Oculus audio switching thing
#include <codecvt>
#endif

#define STUBBED_BASIC() { \
	string str = "Hit stubbed file at " __FILE__ " func "  " line " + to_string(__LINE__); \
	OOVR_ABORT_T(str.c_str(), "Stubbed func!"); \
}

#undef STUBBED
#define STUBBED() { \
	string str = "Hit stubbed file at " __FILE__ " func "  " line " + to_string(__LINE__); \
	str += "via " + string(pchSection) + "." + string(pchSettingsKey); \
	OOVR_ABORT_T(str.c_str(), "Stubbed func!"); \
}

#define UNSET_SETTING() { \
	string str = "Hit undefined setting at " __FILE__ " func "  " line " + to_string(__LINE__); \
	str += "via " + string(pchSection) + "." + string(pchSettingsKey); \
	OOVR_ABORT_T(str.c_str(), "Stubbed func!"); \
}

using namespace std;
namespace kk1 = vr::IVRSettings_001;
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

	string section = pchSection;
	string key = pchSettingsKey;
	if (section == kk::k_pch_SteamVR_Section) {
		if (key == kk::k_pch_SteamVR_ShowStage_Bool) {
			// AFAIK we can just ignore this, it shows a box on the floor.
			return;
		}
		else if (key == kk::k_pch_SteamVR_ForceFadeOnBadTracking_Bool) {
			// Ignore this, it's very unlikely the user will have tracking problems inside their guardian boundries.
			return;
		}
		else if (key == kk::k_pch_SteamVR_BackgroundUseDomeProjection_Bool) {
			// We don't have overlay (which == background?) support
			return;
		}
	}
	else if (section == kk::k_pch_Notifications_Section) {
		if (key == kk::k_pch_Notifications_DoNotDisturb_Bool) {
			// AFAIK there's no way to handle this with LibOVR
			return;
		}
	}
	else if (section == kk::k_pch_CollisionBounds_Section) {
		// No way to alter guardian from LibOVR
		if (key == kk::k_pch_CollisionBounds_GroundPerimeterOn_Bool) {
			return;
		} else if (key == kk::k_pch_CollisionBounds_CenterMarkerOn_Bool) {
			return;
		}
	}
	else if (section == kk::k_pch_Dashboard_Section) {
		if (key == kk::k_pch_Dashboard_EnableDashboard_Bool) {
			// OpenOVR doesn't have a dashboard
			return;
		}
	}

	OOVR_LOG(to_string(bValue).c_str());
	STUBBED();
}
void  BaseSettings::SetInt32(const char * pchSection, const char * pchSettingsKey, int32_t nValue, EVRSettingsError * peError) {
	if (peError)
		*peError = VRSettingsError_None;

	string section = pchSection;
	string key = pchSettingsKey;
	if (section == kk::k_pch_CollisionBounds_Section) {
		if (key == kk::k_pch_CollisionBounds_ColorGammaA_Int32) {
			// AFAIK you can't change the opacity of Guardian, however you can change it's colour
			return;
		}
	}

	STUBBED();
}
void  BaseSettings::SetFloat(const char * pchSection, const char * pchSettingsKey, float flValue, EVRSettingsError * peError) {
	if (peError)
		*peError = VRSettingsError_None;

	string section = pchSection;
	string key = pchSettingsKey;
	if (section == kk::k_pch_CollisionBounds_Section) {
		if (key == kk::k_pch_CollisionBounds_FadeDistance_Float) {
			// No way to alter guardian from LibOVR
			return;
		}
	}
	
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
		else if (key == kk1::k_pch_SteamVR_DirectMode_Bool) {
			// Note that this is NOT the same as k_pch_DirectMode_Section - the key is very slightly different
			// direct_mode vs directMode

			// Oculus doesn't support windowed mode
			return true;
		}
		else if (key == kk::k_pch_SteamVR_RetailDemo_Bool) {
			// What? (Used in The Lab btw)
			return false;
		}
		else if (key == kk1::k_pch_SteamVR_AllowReprojection_Bool || key == "allowInterleavedReprojection") {
			// There were two different reprojection strings for the same property - allowReprojection and allowInterleavedReprojection, however the key was removed
			// at some point, so it's currently just specified as a string. TODO modify the header splitter to keep these old properties around somewhere.
			return true;
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
		if (key == kk::k_pch_SteamVR_SupersampleScale_Float || key == kk1::k_pch_SteamVR_RenderTargetMultiplier_Float) {
			return oovr_global_configuration.SupersampleRatio();
		}
		else if (key == kk::k_pch_SteamVR_IPD_Float) {
			return BaseSystem::SGetIpd();
		}
	} 
	else if (section == kk::k_pch_CollisionBounds_Section)
	{
		if (key == kk::k_pch_CollisionBounds_FadeDistance_Float) {
			return 1.0f; // made up some value that we will not use
		}
	}

	UNSET_SETTING();
}
void  BaseSettings::GetString(const char * pchSection, const char * pchSettingsKey, VR_OUT_STRING() char * pchValue,
	uint32_t unValueLen, EVRSettingsError * peError) {

	if (peError)
		*peError = VRSettingsError_None;

	string section = pchSection;
	string key = pchSettingsKey;

	string result;

	if (section == kk::k_pch_SteamVR_Section) {
		if (key == kk::k_pch_SteamVR_GridColor_String) {
			// When I tested it under SteamVR, it did actually just return an empty string
			result = "";
			goto found;
		}
	}
	else if (section == kk::k_pch_audio_Section) {
#ifdef OC_XR_PORT
		STUBBED();
#else
		// Sansar, and hopefully other games (since this very nicely solves the audio device problem), uses the
		//  auto-switching SteamVR audio devices.
		// See https://gitlab.com/znixian/OpenOVR/issues/65

		wstring_convert<codecvt_utf8<wchar_t>> conv;

		if (key == kk1::k_pch_audio_OnPlaybackDevice_String) {
			wchar_t buff[OVR_AUDIO_MAX_DEVICE_STR_SIZE];
			ovr_GetAudioDeviceOutGuidStr(buff);
			result = conv.to_bytes(buff);
			goto found;
		}
		else if (key == kk1::k_pch_audio_OnRecordDevice_String) {
			wchar_t buff[OVR_AUDIO_MAX_DEVICE_STR_SIZE];
			ovr_GetAudioDeviceInGuidStr(buff);
			result = conv.to_bytes(buff);
			goto found;
		}
#endif
	}

	STUBBED();

found:

	// +1 for the null
	if (unValueLen < result.length() + 1) {
		OOVR_ABORT("unValueLen too short!");
	}

	strcpy_s(pchValue, unValueLen, result.c_str());
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
