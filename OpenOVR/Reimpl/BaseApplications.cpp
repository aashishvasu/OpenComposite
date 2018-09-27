#include "stdafx.h"
#define BASE_IMPL
#include "BaseApplications.h"

using namespace std;
using namespace vr;

/** The maximum length of an application key */
static const uint32_t k_unMaxApplicationKeyLength = 128;

// So I can easily use it for return types
typedef OOVR_EVRApplicationError EVRApplicationError;

EVRApplicationError BaseApplications::AddApplicationManifest(const char *pchApplicationManifestFullPath, bool bTemporary) {
	STUBBED();
}
EVRApplicationError BaseApplications::RemoveApplicationManifest(const char *pchApplicationManifestFullPath) {
	STUBBED();
}
bool BaseApplications::IsApplicationInstalled(const char *pchAppKey) {
	STUBBED();
}
uint32_t BaseApplications::GetApplicationCount() {
	STUBBED();
}
EVRApplicationError BaseApplications::GetApplicationKeyByIndex(uint32_t unApplicationIndex, VR_OUT_STRING() char *pchAppKeyBuffer, uint32_t unAppKeyBufferLen) {
	STUBBED();
}
EVRApplicationError BaseApplications::GetApplicationKeyByProcessId(uint32_t unProcessId, VR_OUT_STRING() char *pchAppKeyBuffer, uint32_t unAppKeyBufferLen) {
	STUBBED();
}
EVRApplicationError BaseApplications::LaunchApplication(const char *pchAppKey) {
	STUBBED();
}
EVRApplicationError BaseApplications::LaunchTemplateApplication(const char *pchTemplateAppKey, const char *pchNewAppKey, VR_ARRAY_COUNT(unKeys) const AppOverrideKeys_t *pKeys, uint32_t unKeys) {
	STUBBED();
}
EVRApplicationError BaseApplications::LaunchApplicationFromMimeType(const char *pchMimeType, const char *pchArgs) {
	STUBBED();
}
EVRApplicationError BaseApplications::LaunchDashboardOverlay(const char *pchAppKey) {
	STUBBED();
}
bool BaseApplications::CancelApplicationLaunch(const char *pchAppKey) {
	STUBBED();
}
EVRApplicationError BaseApplications::IdentifyApplication(uint32_t unProcessId, const char *pchAppKey) {
	STUBBED();
}
uint32_t BaseApplications::GetApplicationProcessId(const char *pchAppKey) {
	STUBBED();
}
const char * BaseApplications::GetApplicationsErrorNameFromEnum(EVRApplicationError error) {
	STUBBED();
}
uint32_t BaseApplications::GetApplicationPropertyString(const char *pchAppKey, EVRApplicationProperty eProperty, VR_OUT_STRING() char *pchPropertyValueBuffer, uint32_t unPropertyValueBufferLen, EVRApplicationError *peError) {
	STUBBED();
}
bool BaseApplications::GetApplicationPropertyBool(const char *pchAppKey, EVRApplicationProperty eProperty, EVRApplicationError *peError) {
	STUBBED();
}
uint64_t BaseApplications::GetApplicationPropertyUint64(const char *pchAppKey, EVRApplicationProperty eProperty, EVRApplicationError *peError) {
	STUBBED();
}
EVRApplicationError BaseApplications::SetApplicationAutoLaunch(const char *pchAppKey, bool bAutoLaunch) {
	STUBBED();
}
bool BaseApplications::GetApplicationAutoLaunch(const char *pchAppKey) {
	STUBBED();
}
EVRApplicationError BaseApplications::SetDefaultApplicationForMimeType(const char *pchAppKey, const char *pchMimeType) {
	STUBBED();
}
bool BaseApplications::GetDefaultApplicationForMimeType(const char *pchMimeType, VR_OUT_STRING() char *pchAppKeyBuffer, uint32_t unAppKeyBufferLen) {
	STUBBED();
}
bool BaseApplications::GetApplicationSupportedMimeTypes(const char *pchAppKey, VR_OUT_STRING() char *pchMimeTypesBuffer, uint32_t unMimeTypesBuffer) {
	STUBBED();
}
uint32_t BaseApplications::GetApplicationsThatSupportMimeType(const char *pchMimeType, VR_OUT_STRING() char *pchAppKeysThatSupportBuffer, uint32_t unAppKeysThatSupportBuffer) {
	STUBBED();
}
uint32_t BaseApplications::GetApplicationLaunchArguments(uint32_t unHandle, VR_OUT_STRING() char *pchArgs, uint32_t unArgs) {
	STUBBED();
}
EVRApplicationError BaseApplications::GetStartingApplication(VR_OUT_STRING() char *pchAppKeyBuffer, uint32_t unAppKeyBufferLen) {
	STUBBED();
}
BaseApplications::EVRApplicationTransitionState BaseApplications::GetTransitionState() {
	STUBBED();
}
EVRApplicationError BaseApplications::PerformApplicationPrelaunchCheck(const char *pchAppKey) {
	STUBBED();
}
const char * BaseApplications::GetApplicationsTransitionStateNameFromEnum(EVRApplicationTransitionState state) {
	STUBBED();
}
bool BaseApplications::IsQuitUserPromptRequested() {
	STUBBED();
}
EVRApplicationError BaseApplications::LaunchInternalProcess(const char *pchBinaryPath, const char *pchArguments, const char *pchWorkingDirectory) {
	STUBBED();
}
uint32_t BaseApplications::GetCurrentSceneProcessId() {
	STUBBED();
}
