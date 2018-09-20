#include "stdafx.h"
#define CVR_IMPL
#include "CVRScreenshots001.h"

CVR_GEN_IMPL(CVRScreenshots_001);

using namespace vr;

using berr_t = BaseScreenshots::EVRScreenshotError;
using verr_t = IVRScreenshots_001::EVRScreenshotError;

verr_t CVRScreenshots_001::RequestScreenshot(ScreenshotHandle_t * pOutScreenshotHandle, EVRScreenshotType type, const char * pchPreviewFilename, const char * pchVRFilename) {
	return (verr_t)base.RequestScreenshot(pOutScreenshotHandle, type, pchPreviewFilename, pchVRFilename);
}
verr_t CVRScreenshots_001::HookScreenshot(const EVRScreenshotType * pSupportedTypes, int numTypes) {
	return (verr_t)base.HookScreenshot(pSupportedTypes, numTypes);
}
EVRScreenshotType CVRScreenshots_001::GetScreenshotPropertyType(ScreenshotHandle_t screenshotHandle, verr_t * pError) {
	return base.GetScreenshotPropertyType(screenshotHandle, (berr_t*)pError);
}
uint32_t CVRScreenshots_001::GetScreenshotPropertyFilename(ScreenshotHandle_t screenshotHandle, EVRScreenshotPropertyFilenames filenameType, char * pchFilename, uint32_t cchFilename, verr_t * pError) {
	return base.GetScreenshotPropertyFilename(screenshotHandle, filenameType, pchFilename, cchFilename, (berr_t*) pError);
}
verr_t CVRScreenshots_001::UpdateScreenshotProgress(ScreenshotHandle_t screenshotHandle, float flProgress) {
	return (verr_t)base.UpdateScreenshotProgress(screenshotHandle, flProgress);
}
verr_t CVRScreenshots_001::TakeStereoScreenshot(ScreenshotHandle_t * pOutScreenshotHandle, const char * pchPreviewFilename, const char * pchVRFilename) {
	return (verr_t)base.TakeStereoScreenshot(pOutScreenshotHandle, pchPreviewFilename, pchVRFilename);
}
verr_t CVRScreenshots_001::SubmitScreenshot(ScreenshotHandle_t screenshotHandle, EVRScreenshotType type, const char * pchSourcePreviewFilename, const char * pchSourceVRFilename) {
	return (verr_t)base.SubmitScreenshot(screenshotHandle, type, pchSourcePreviewFilename, pchSourceVRFilename);
}
