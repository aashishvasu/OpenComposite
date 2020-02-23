#include "stdafx.h"
#define GENFILE
#include "BaseCommon.h"

GEN_INTERFACE("Settings", "001")
GEN_INTERFACE("Settings", "002")
GEN_INTERFACE("Settings", "003")

#include "GVRSettings.gen.h"

typedef BaseSettings::EVRSettingsError EVRSettingsErrorBase;
typedef vr::IVRSettings_001::EVRSettingsError EVRSettingsError001;

bool CVRSettings_001::GetBool(const char *pchSection, const char *pchSettingsKey, bool bDefaultValue, EVRSettingsError001 *peError) {
	return base->GetBool(pchSection, pchSettingsKey, (EVRSettingsErrorBase*)peError);

}
int32_t CVRSettings_001::GetInt32(const char *pchSection, const char *pchSettingsKey, int32_t nDefaultValue, EVRSettingsError001 *peError) {
	return base->GetInt32(pchSection, pchSettingsKey, (EVRSettingsErrorBase*)peError);
};

float CVRSettings_001::GetFloat(const char *pchSection, const char *pchSettingsKey, float flDefaultValue, EVRSettingsError001 *peError) {
	return base->GetFloat(pchSection, pchSettingsKey, (EVRSettingsErrorBase*)peError);
};

void CVRSettings_001::GetString(const char *pchSection, const char *pchSettingsKey, char *pchValue, uint32_t unValueLen,
		const char *pchDefaultValue, EVRSettingsError001 *peError) {

	base->GetString(pchSection, pchSettingsKey, pchValue, unValueLen, (EVRSettingsErrorBase*) peError);
};
