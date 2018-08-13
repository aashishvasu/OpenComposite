#include "stdafx.h"
#define CVR_IMPL
#include "CVRSettings002.h"
CVR_GEN_IMPL(CVRSettings_002);

using err_t = BaseSettings::EVRSettingsError;

const char * CVRSettings_002::GetSettingsErrorNameFromEnum(EVRSettingsError eError) {
	return base.GetSettingsErrorNameFromEnum((err_t) eError);
}
bool CVRSettings_002::Sync(bool bForce, EVRSettingsError * peError) {
	return base.Sync(bForce, (err_t*)peError);
}
void CVRSettings_002::SetBool(const char * pchSection, const char * pchSettingsKey, bool bValue, EVRSettingsError * peError) {
	return base.SetBool(pchSection, pchSettingsKey, bValue, (err_t*)peError);
}
void CVRSettings_002::SetInt32(const char * pchSection, const char * pchSettingsKey, int32_t nValue, EVRSettingsError * peError) {
	return base.SetInt32(pchSection, pchSettingsKey, nValue, (err_t*)peError);
}
void CVRSettings_002::SetFloat(const char * pchSection, const char * pchSettingsKey, float flValue, EVRSettingsError * peError) {
	return base.SetFloat(pchSection, pchSettingsKey, flValue, (err_t*)peError);
}
void CVRSettings_002::SetString(const char * pchSection, const char * pchSettingsKey, const char * pchValue, EVRSettingsError * peError) {
	return base.SetString(pchSection, pchSettingsKey, pchValue, (err_t*)peError);
}
bool CVRSettings_002::GetBool(const char * pchSection, const char * pchSettingsKey, EVRSettingsError * peError) {
	return base.GetBool(pchSection, pchSettingsKey, (err_t*)peError);
}
int32_t CVRSettings_002::GetInt32(const char * pchSection, const char * pchSettingsKey, EVRSettingsError * peError) {
	return base.GetInt32(pchSection, pchSettingsKey, (err_t*)peError);
}
float CVRSettings_002::GetFloat(const char * pchSection, const char * pchSettingsKey, EVRSettingsError * peError) {
	return base.GetFloat(pchSection, pchSettingsKey, (err_t*)peError);
}
void CVRSettings_002::GetString(const char * pchSection, const char * pchSettingsKey, VR_OUT_STRING() char * pchValue, uint32_t unValueLen, EVRSettingsError * peError) {
	return base.GetString(pchSection, pchSettingsKey, pchValue, unValueLen, (err_t*)peError);
}
void CVRSettings_002::RemoveSection(const char * pchSection, EVRSettingsError * peError) {
	return base.RemoveSection(pchSection, (err_t*)peError);
}
void CVRSettings_002::RemoveKeyInSection(const char * pchSection, const char * pchSettingsKey, EVRSettingsError * peError) {
	return base.RemoveKeyInSection(pchSection, pchSettingsKey, (err_t*)peError);
}
