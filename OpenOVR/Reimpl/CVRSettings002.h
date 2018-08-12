#pragma once
#include "BaseSettings.h"
#include "OpenVR/interfaces/IVRSettings_002.h"

using namespace vr; // TODO remove

class CVRSettings_002 : public IVRSettings_002::IVRSettings {
private:
	BaseSettings base;
	using EVRSettingsError = IVRSettings_002::EVRSettingsError;
public:
	INTERFACE_FUNC(const char *, GetSettingsErrorNameFromEnum, EVRSettingsError eError);

	// Returns true if file sync occurred (force or settings dirty)
	INTERFACE_FUNC(bool, Sync, bool bForce = false, EVRSettingsError *peError = nullptr);

	INTERFACE_FUNC(void, SetBool, const char *pchSection, const char *pchSettingsKey, bool bValue, EVRSettingsError *peError = nullptr);
	INTERFACE_FUNC(void, SetInt32, const char *pchSection, const char *pchSettingsKey, int32_t nValue, EVRSettingsError *peError = nullptr);
	INTERFACE_FUNC(void, SetFloat, const char *pchSection, const char *pchSettingsKey, float flValue, EVRSettingsError *peError = nullptr);
	INTERFACE_FUNC(void, SetString, const char *pchSection, const char *pchSettingsKey, const char *pchValue, EVRSettingsError *peError = nullptr);

	// Users of the system need to provide a proper default in default.vrsettings in the resources/settings/ directory
	// of either the runtime or the driver_xxx directory. Otherwise the default will be false, 0, 0.0 or ""
	INTERFACE_FUNC(bool, GetBool, const char *pchSection, const char *pchSettingsKey, EVRSettingsError *peError = nullptr);
	INTERFACE_FUNC(int32_t, GetInt32, const char *pchSection, const char *pchSettingsKey, EVRSettingsError *peError = nullptr);
	INTERFACE_FUNC(float, GetFloat, const char *pchSection, const char *pchSettingsKey, EVRSettingsError *peError = nullptr);
	INTERFACE_FUNC(void, GetString, const char *pchSection, const char *pchSettingsKey, VR_OUT_STRING() char *pchValue, uint32_t unValueLen, EVRSettingsError *peError = nullptr);

	INTERFACE_FUNC(void, RemoveSection, const char *pchSection, EVRSettingsError *peError = nullptr);
	INTERFACE_FUNC(void, RemoveKeyInSection, const char *pchSection, const char *pchSettingsKey, EVRSettingsError *peError = nullptr);
};
