#include "stdafx.h"
#define BASE_IMPL
#include "BaseScreenshots.h"
#include <string>

using namespace vr;
using EVRScreenshotError = BaseScreenshots::EVRScreenshotError;

EVRScreenshotError BaseScreenshots::RequestScreenshot(ScreenshotHandle_t *pOutScreenshotHandle, EVRScreenshotType type, const char *pchPreviewFilename, const char *pchVRFilename) {
	STUBBED();
}
EVRScreenshotError BaseScreenshots::HookScreenshot(VR_ARRAY_COUNT(numTypes) const EVRScreenshotType *pSupportedTypes, int numTypes) {
	// Since AFAIK LibOVR doesn't give the player the ability to trigger screenshots, do nothing here.
	return VRScreenshotError_None;
}
EVRScreenshotType BaseScreenshots::GetScreenshotPropertyType(ScreenshotHandle_t screenshotHandle, EVRScreenshotError *pError) {
	if (pError)
		*pError = VRScreenshotError_None;

	STUBBED();
}
uint32_t BaseScreenshots::GetScreenshotPropertyFilename(ScreenshotHandle_t screenshotHandle, EVRScreenshotPropertyFilenames filenameType, VR_OUT_STRING() char *pchFilename, uint32_t cchFilename, EVRScreenshotError *pError) {
	if (pError)
		*pError = VRScreenshotError_None;

	STUBBED();
}
EVRScreenshotError BaseScreenshots::UpdateScreenshotProgress(ScreenshotHandle_t screenshotHandle, float flProgress) {
	STUBBED();
}
EVRScreenshotError BaseScreenshots::TakeStereoScreenshot(ScreenshotHandle_t *pOutScreenshotHandle, const char *pchPreviewFilename, const char *pchVRFilename) {
	STUBBED();
}
EVRScreenshotError BaseScreenshots::SubmitScreenshot(ScreenshotHandle_t screenshotHandle, EVRScreenshotType type, const char *pchSourcePreviewFilename, const char *pchSourceVRFilename) {
	STUBBED();
}
