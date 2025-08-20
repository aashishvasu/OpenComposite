#include "stdafx.h"
#define BASE_IMPL
#include "BaseScreenshots.h"
#include <string>

using namespace vr;
using EVRScreenshotError = BaseScreenshots::EVRScreenshotError;

EVRScreenshotError BaseScreenshots::RequestScreenshot(ScreenshotHandle_t* pOutScreenshotHandle, EVRScreenshotType type, const char* pchPreviewFilename, const char* pchVRFilename)
{
	return VRScreenshotError_IncompatibleVersion;
}
EVRScreenshotError BaseScreenshots::HookScreenshot(VR_ARRAY_COUNT(numTypes) const EVRScreenshotType* pSupportedTypes, int numTypes)
{
	return VRScreenshotError_IncompatibleVersion;
}
EVRScreenshotType BaseScreenshots::GetScreenshotPropertyType(ScreenshotHandle_t screenshotHandle, EVRScreenshotError* pError)
{
	if (pError)
		*pError = VRScreenshotError_IncompatibleVersion;

	return (EVRScreenshotType)0;
}
uint32_t BaseScreenshots::GetScreenshotPropertyFilename(ScreenshotHandle_t screenshotHandle, EVRScreenshotPropertyFilenames filenameType, VR_OUT_STRING() char* pchFilename, uint32_t cchFilename, EVRScreenshotError* pError)
{
	if (pError)
		*pError = VRScreenshotError_IncompatibleVersion;

	return 0;
}
EVRScreenshotError BaseScreenshots::UpdateScreenshotProgress(ScreenshotHandle_t screenshotHandle, float flProgress)
{
	return VRScreenshotError_IncompatibleVersion;
}
EVRScreenshotError BaseScreenshots::TakeStereoScreenshot(ScreenshotHandle_t* pOutScreenshotHandle, const char* pchPreviewFilename, const char* pchVRFilename)
{
	return VRScreenshotError_IncompatibleVersion;
}
EVRScreenshotError BaseScreenshots::SubmitScreenshot(ScreenshotHandle_t screenshotHandle, EVRScreenshotType type, const char* pchSourcePreviewFilename, const char* pchSourceVRFilename)
{
	return VRScreenshotError_IncompatibleVersion;
}
