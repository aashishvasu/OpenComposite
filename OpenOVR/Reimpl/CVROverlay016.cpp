#include "stdafx.h"
#define CVR_IMPL
#include "CVROverlay016.h"
CVR_GEN_IMPL(CVROverlay_016);

using namespace IVROverlay_016;

#include <string>
#define STUBBED() { \
	std::string str = "Hit stubbed file at " __FILE__ " func "  " line " + std::to_string(__LINE__); \
	MessageBoxA(NULL, str.c_str(), "Stubbed func!", MB_OK); \
	throw "stub"; \
}

EVROverlayError CVROverlay_016::FindOverlay(const char * pchOverlayKey, VROverlayHandle_t * pOverlayHandle) {
	return base.FindOverlay(pchOverlayKey, pOverlayHandle);
}
EVROverlayError CVROverlay_016::CreateOverlay(const char * pchOverlayKey, const char * pchOverlayName, VROverlayHandle_t * pOverlayHandle) {
	return base.CreateOverlay(pchOverlayKey, pchOverlayName, pOverlayHandle);
}
EVROverlayError CVROverlay_016::DestroyOverlay(VROverlayHandle_t ulOverlayHandle) {
	return base.DestroyOverlay(ulOverlayHandle);
}
EVROverlayError CVROverlay_016::SetHighQualityOverlay(VROverlayHandle_t ulOverlayHandle) {
	return base.SetHighQualityOverlay(ulOverlayHandle);
}
VROverlayHandle_t CVROverlay_016::GetHighQualityOverlay() {
	return base.GetHighQualityOverlay();
}
uint32_t CVROverlay_016::GetOverlayKey(VROverlayHandle_t ulOverlayHandle, VR_OUT_STRING() char * pchValue, uint32_t unBufferSize, EVROverlayError * pError) {
	return base.GetOverlayKey(ulOverlayHandle, pchValue, unBufferSize, pError);
}
uint32_t CVROverlay_016::GetOverlayName(VROverlayHandle_t ulOverlayHandle, VR_OUT_STRING() char * pchValue, uint32_t unBufferSize, EVROverlayError * pError) {
	return base.GetOverlayName(ulOverlayHandle, pchValue, unBufferSize, pError);
}
EVROverlayError CVROverlay_016::SetOverlayName(VROverlayHandle_t ulOverlayHandle, const char * pchName) {
	return base.SetOverlayName(ulOverlayHandle, pchName);
}
EVROverlayError CVROverlay_016::GetOverlayImageData(VROverlayHandle_t ulOverlayHandle, void * pvBuffer, uint32_t unBufferSize, uint32_t * punWidth, uint32_t * punHeight) {
	return base.GetOverlayImageData(ulOverlayHandle, pvBuffer, unBufferSize, punWidth, punHeight);
}
const char * CVROverlay_016::GetOverlayErrorNameFromEnum(EVROverlayError error) {
	return base.GetOverlayErrorNameFromEnum(error);
}
EVROverlayError CVROverlay_016::SetOverlayRenderingPid(VROverlayHandle_t ulOverlayHandle, uint32_t unPID) {
	return base.SetOverlayRenderingPid(ulOverlayHandle, unPID);
}
uint32_t CVROverlay_016::GetOverlayRenderingPid(VROverlayHandle_t ulOverlayHandle) {
	return base.GetOverlayRenderingPid(ulOverlayHandle);
}
EVROverlayError CVROverlay_016::SetOverlayFlag(VROverlayHandle_t ulOverlayHandle, VROverlayFlags eOverlayFlag, bool bEnabled) {
	return base.SetOverlayFlag(ulOverlayHandle, eOverlayFlag, bEnabled);
}
EVROverlayError CVROverlay_016::GetOverlayFlag(VROverlayHandle_t ulOverlayHandle, VROverlayFlags eOverlayFlag, bool * pbEnabled) {
	return base.GetOverlayFlag(ulOverlayHandle, eOverlayFlag, pbEnabled);
}
EVROverlayError CVROverlay_016::SetOverlayColor(VROverlayHandle_t ulOverlayHandle, float fRed, float fGreen, float fBlue) {
	return base.SetOverlayColor(ulOverlayHandle, fRed, fGreen, fBlue);
}
EVROverlayError CVROverlay_016::GetOverlayColor(VROverlayHandle_t ulOverlayHandle, float * pfRed, float * pfGreen, float * pfBlue) {
	return base.GetOverlayColor(ulOverlayHandle, pfRed, pfGreen, pfBlue);
}
EVROverlayError CVROverlay_016::SetOverlayAlpha(VROverlayHandle_t ulOverlayHandle, float fAlpha) {
	return base.SetOverlayAlpha(ulOverlayHandle, fAlpha);
}
EVROverlayError CVROverlay_016::GetOverlayAlpha(VROverlayHandle_t ulOverlayHandle, float * pfAlpha) {
	return base.GetOverlayAlpha(ulOverlayHandle, pfAlpha);
}
EVROverlayError CVROverlay_016::SetOverlayTexelAspect(VROverlayHandle_t ulOverlayHandle, float fTexelAspect) {
	return base.SetOverlayTexelAspect(ulOverlayHandle, fTexelAspect);
}
EVROverlayError CVROverlay_016::GetOverlayTexelAspect(VROverlayHandle_t ulOverlayHandle, float * pfTexelAspect) {
	return base.GetOverlayTexelAspect(ulOverlayHandle, pfTexelAspect);
}
EVROverlayError CVROverlay_016::SetOverlaySortOrder(VROverlayHandle_t ulOverlayHandle, uint32_t unSortOrder) {
	return base.SetOverlaySortOrder(ulOverlayHandle, unSortOrder);
}
EVROverlayError CVROverlay_016::GetOverlaySortOrder(VROverlayHandle_t ulOverlayHandle, uint32_t * punSortOrder) {
	return base.GetOverlaySortOrder(ulOverlayHandle, punSortOrder);
}
EVROverlayError CVROverlay_016::SetOverlayWidthInMeters(VROverlayHandle_t ulOverlayHandle, float fWidthInMeters) {
	return base.SetOverlayWidthInMeters(ulOverlayHandle, fWidthInMeters);
}
EVROverlayError CVROverlay_016::GetOverlayWidthInMeters(VROverlayHandle_t ulOverlayHandle, float * pfWidthInMeters) {
	return base.GetOverlayWidthInMeters(ulOverlayHandle, pfWidthInMeters);
}
EVROverlayError CVROverlay_016::SetOverlayAutoCurveDistanceRangeInMeters(VROverlayHandle_t ulOverlayHandle, float fMinDistanceInMeters, float fMaxDistanceInMeters) {
	return base.SetOverlayAutoCurveDistanceRangeInMeters(ulOverlayHandle, fMinDistanceInMeters, fMaxDistanceInMeters);
}
EVROverlayError CVROverlay_016::GetOverlayAutoCurveDistanceRangeInMeters(VROverlayHandle_t ulOverlayHandle, float * pfMinDistanceInMeters, float * pfMaxDistanceInMeters) {
	return base.GetOverlayAutoCurveDistanceRangeInMeters(ulOverlayHandle, pfMinDistanceInMeters, pfMaxDistanceInMeters);
}
EVROverlayError CVROverlay_016::SetOverlayTextureColorSpace(VROverlayHandle_t ulOverlayHandle, EColorSpace eTextureColorSpace) {
	return base.SetOverlayTextureColorSpace(ulOverlayHandle, eTextureColorSpace);
}
EVROverlayError CVROverlay_016::GetOverlayTextureColorSpace(VROverlayHandle_t ulOverlayHandle, EColorSpace * peTextureColorSpace) {
	return base.GetOverlayTextureColorSpace(ulOverlayHandle, peTextureColorSpace);
}
EVROverlayError CVROverlay_016::SetOverlayTextureBounds(VROverlayHandle_t ulOverlayHandle, const VRTextureBounds_t * pOverlayTextureBounds) {
	return base.SetOverlayTextureBounds(ulOverlayHandle, pOverlayTextureBounds);
}
EVROverlayError CVROverlay_016::GetOverlayTextureBounds(VROverlayHandle_t ulOverlayHandle, VRTextureBounds_t * pOverlayTextureBounds) {
	return base.GetOverlayTextureBounds(ulOverlayHandle, pOverlayTextureBounds);
}
uint32_t CVROverlay_016::GetOverlayRenderModel(VROverlayHandle_t ulOverlayHandle, char * pchValue, uint32_t unBufferSize, HmdColor_t * pColor, EVROverlayError * pError) {
	return base.GetOverlayRenderModel(ulOverlayHandle, pchValue, unBufferSize, pColor, pError);
}
EVROverlayError CVROverlay_016::SetOverlayRenderModel(VROverlayHandle_t ulOverlayHandle, const char * pchRenderModel, const HmdColor_t * pColor) {
	return base.SetOverlayRenderModel(ulOverlayHandle, pchRenderModel, pColor);
}
EVROverlayError CVROverlay_016::GetOverlayTransformType(VROverlayHandle_t ulOverlayHandle, VROverlayTransformType * peTransformType) {
	return base.GetOverlayTransformType(ulOverlayHandle, (int*)peTransformType);
}
EVROverlayError CVROverlay_016::SetOverlayTransformAbsolute(VROverlayHandle_t ulOverlayHandle, ETrackingUniverseOrigin eTrackingOrigin, const HmdMatrix34_t * pmatTrackingOriginToOverlayTransform) {
	return base.SetOverlayTransformAbsolute(ulOverlayHandle, eTrackingOrigin, pmatTrackingOriginToOverlayTransform);
}
EVROverlayError CVROverlay_016::GetOverlayTransformAbsolute(VROverlayHandle_t ulOverlayHandle, ETrackingUniverseOrigin * peTrackingOrigin, HmdMatrix34_t * pmatTrackingOriginToOverlayTransform) {
	return base.GetOverlayTransformAbsolute(ulOverlayHandle, peTrackingOrigin, pmatTrackingOriginToOverlayTransform);
}
EVROverlayError CVROverlay_016::SetOverlayTransformTrackedDeviceRelative(VROverlayHandle_t ulOverlayHandle, TrackedDeviceIndex_t unTrackedDevice, const HmdMatrix34_t * pmatTrackedDeviceToOverlayTransform) {
	return base.SetOverlayTransformTrackedDeviceRelative(ulOverlayHandle, unTrackedDevice, pmatTrackedDeviceToOverlayTransform);
}
EVROverlayError CVROverlay_016::GetOverlayTransformTrackedDeviceRelative(VROverlayHandle_t ulOverlayHandle, TrackedDeviceIndex_t * punTrackedDevice, HmdMatrix34_t * pmatTrackedDeviceToOverlayTransform) {
	return base.GetOverlayTransformTrackedDeviceRelative(ulOverlayHandle, punTrackedDevice, pmatTrackedDeviceToOverlayTransform);
}
EVROverlayError CVROverlay_016::SetOverlayTransformTrackedDeviceComponent(VROverlayHandle_t ulOverlayHandle, TrackedDeviceIndex_t unDeviceIndex, const char * pchComponentName) {
	return base.SetOverlayTransformTrackedDeviceComponent(ulOverlayHandle, unDeviceIndex, pchComponentName);
}
EVROverlayError CVROverlay_016::GetOverlayTransformTrackedDeviceComponent(VROverlayHandle_t ulOverlayHandle, TrackedDeviceIndex_t * punDeviceIndex, char * pchComponentName, uint32_t unComponentNameSize) {
	return base.GetOverlayTransformTrackedDeviceComponent(ulOverlayHandle, punDeviceIndex, pchComponentName, unComponentNameSize);
}
EVROverlayError CVROverlay_016::GetOverlayTransformOverlayRelative(VROverlayHandle_t ulOverlayHandle, VROverlayHandle_t * ulOverlayHandleParent, HmdMatrix34_t * pmatParentOverlayToOverlayTransform) {
	return base.GetOverlayTransformOverlayRelative(ulOverlayHandle, ulOverlayHandleParent, pmatParentOverlayToOverlayTransform);
}
EVROverlayError CVROverlay_016::SetOverlayTransformOverlayRelative(VROverlayHandle_t ulOverlayHandle, VROverlayHandle_t ulOverlayHandleParent, const HmdMatrix34_t * pmatParentOverlayToOverlayTransform) {
	return base.SetOverlayTransformOverlayRelative(ulOverlayHandle, ulOverlayHandleParent, pmatParentOverlayToOverlayTransform);
}
EVROverlayError CVROverlay_016::ShowOverlay(VROverlayHandle_t ulOverlayHandle) {
	return base.ShowOverlay(ulOverlayHandle);
}
EVROverlayError CVROverlay_016::HideOverlay(VROverlayHandle_t ulOverlayHandle) {
	return base.HideOverlay(ulOverlayHandle);
}
bool CVROverlay_016::IsOverlayVisible(VROverlayHandle_t ulOverlayHandle) {
	return base.IsOverlayVisible(ulOverlayHandle);
}
EVROverlayError CVROverlay_016::GetTransformForOverlayCoordinates(VROverlayHandle_t ulOverlayHandle, ETrackingUniverseOrigin eTrackingOrigin, HmdVector2_t coordinatesInOverlay, HmdMatrix34_t * pmatTransform) {
	return base.GetTransformForOverlayCoordinates(ulOverlayHandle, eTrackingOrigin, coordinatesInOverlay, pmatTransform);
}
bool CVROverlay_016::PollNextOverlayEvent(VROverlayHandle_t ulOverlayHandle, VREvent_t * pEvent, uint32_t uncbVREvent) {
	return base.PollNextOverlayEvent(ulOverlayHandle, pEvent, uncbVREvent);
}
EVROverlayError CVROverlay_016::GetOverlayInputMethod(VROverlayHandle_t ulOverlayHandle, VROverlayInputMethod * peInputMethod) {
	return base.GetOverlayInputMethod(ulOverlayHandle, (int*)peInputMethod);
}
EVROverlayError CVROverlay_016::SetOverlayInputMethod(VROverlayHandle_t ulOverlayHandle, VROverlayInputMethod eInputMethod) {
	return base.SetOverlayInputMethod(ulOverlayHandle, eInputMethod);
}
EVROverlayError CVROverlay_016::GetOverlayMouseScale(VROverlayHandle_t ulOverlayHandle, HmdVector2_t * pvecMouseScale) {
	return base.GetOverlayMouseScale(ulOverlayHandle, pvecMouseScale);
}
EVROverlayError CVROverlay_016::SetOverlayMouseScale(VROverlayHandle_t ulOverlayHandle, const HmdVector2_t * pvecMouseScale) {
	return base.SetOverlayMouseScale(ulOverlayHandle, pvecMouseScale);
}
bool CVROverlay_016::ComputeOverlayIntersection(VROverlayHandle_t ulOverlayHandle, const VROverlayIntersectionParams_t * pParams, VROverlayIntersectionResults_t * pResults) {
	//return base.ComputeOverlayIntersection(ulOverlayHandle, pParams, pResults);
	STUBBED();
}
bool CVROverlay_016::HandleControllerOverlayInteractionAsMouse(VROverlayHandle_t ulOverlayHandle, TrackedDeviceIndex_t unControllerDeviceIndex) {
	return base.HandleControllerOverlayInteractionAsMouse(ulOverlayHandle, unControllerDeviceIndex);
}
bool CVROverlay_016::IsHoverTargetOverlay(VROverlayHandle_t ulOverlayHandle) {
	return base.IsHoverTargetOverlay(ulOverlayHandle);
}
VROverlayHandle_t CVROverlay_016::GetGamepadFocusOverlay() {
	return base.GetGamepadFocusOverlay();
}
EVROverlayError CVROverlay_016::SetGamepadFocusOverlay(VROverlayHandle_t ulNewFocusOverlay) {
	return base.SetGamepadFocusOverlay(ulNewFocusOverlay);
}
EVROverlayError CVROverlay_016::SetOverlayNeighbor(EOverlayDirection eDirection, VROverlayHandle_t ulFrom, VROverlayHandle_t ulTo) {
	return base.SetOverlayNeighbor(eDirection, ulFrom, ulTo);
}
EVROverlayError CVROverlay_016::MoveGamepadFocusToNeighbor(EOverlayDirection eDirection, VROverlayHandle_t ulFrom) {
	return base.MoveGamepadFocusToNeighbor(eDirection, ulFrom);
}
EVROverlayError CVROverlay_016::SetOverlayTexture(VROverlayHandle_t ulOverlayHandle, const Texture_t * pTexture) {
	return base.SetOverlayTexture(ulOverlayHandle, pTexture);
}
EVROverlayError CVROverlay_016::ClearOverlayTexture(VROverlayHandle_t ulOverlayHandle) {
	return base.ClearOverlayTexture(ulOverlayHandle);
}
EVROverlayError CVROverlay_016::SetOverlayRaw(VROverlayHandle_t ulOverlayHandle, void * pvBuffer, uint32_t unWidth, uint32_t unHeight, uint32_t unDepth) {
	return base.SetOverlayRaw(ulOverlayHandle, pvBuffer, unWidth, unHeight, unDepth);
}
EVROverlayError CVROverlay_016::SetOverlayFromFile(VROverlayHandle_t ulOverlayHandle, const char * pchFilePath) {
	return base.SetOverlayFromFile(ulOverlayHandle, pchFilePath);
}
EVROverlayError CVROverlay_016::GetOverlayTexture(VROverlayHandle_t ulOverlayHandle, void ** pNativeTextureHandle, void * pNativeTextureRef, uint32_t * pWidth, uint32_t * pHeight, uint32_t * pNativeFormat, ETextureType * pAPIType, EColorSpace * pColorSpace, VRTextureBounds_t * pTextureBounds) {
	return base.GetOverlayTexture(ulOverlayHandle, pNativeTextureHandle, pNativeTextureRef, pWidth, pHeight, pNativeFormat, pAPIType, pColorSpace, pTextureBounds);
}
EVROverlayError CVROverlay_016::ReleaseNativeOverlayHandle(VROverlayHandle_t ulOverlayHandle, void * pNativeTextureHandle) {
	return base.ReleaseNativeOverlayHandle(ulOverlayHandle, pNativeTextureHandle);
}
EVROverlayError CVROverlay_016::GetOverlayTextureSize(VROverlayHandle_t ulOverlayHandle, uint32_t * pWidth, uint32_t * pHeight) {
	return base.GetOverlayTextureSize(ulOverlayHandle, pWidth, pHeight);
}
EVROverlayError CVROverlay_016::CreateDashboardOverlay(const char * pchOverlayKey, const char * pchOverlayFriendlyName, VROverlayHandle_t * pMainHandle, VROverlayHandle_t * pThumbnailHandle) {
	return base.CreateDashboardOverlay(pchOverlayKey, pchOverlayFriendlyName, pMainHandle, pThumbnailHandle);
}
bool CVROverlay_016::IsDashboardVisible() {
	return base.IsDashboardVisible();
}
bool CVROverlay_016::IsActiveDashboardOverlay(VROverlayHandle_t ulOverlayHandle) {
	return base.IsActiveDashboardOverlay(ulOverlayHandle);
}
EVROverlayError CVROverlay_016::SetDashboardOverlaySceneProcess(VROverlayHandle_t ulOverlayHandle, uint32_t unProcessId) {
	return base.SetDashboardOverlaySceneProcess(ulOverlayHandle, unProcessId);
}
EVROverlayError CVROverlay_016::GetDashboardOverlaySceneProcess(VROverlayHandle_t ulOverlayHandle, uint32_t * punProcessId) {
	return base.GetDashboardOverlaySceneProcess(ulOverlayHandle, punProcessId);
}
void CVROverlay_016::ShowDashboard(const char * pchOverlayToShow) {
	return base.ShowDashboard(pchOverlayToShow);
}
TrackedDeviceIndex_t CVROverlay_016::GetPrimaryDashboardDevice() {
	return base.GetPrimaryDashboardDevice();
}
EVROverlayError CVROverlay_016::ShowKeyboard(EGamepadTextInputMode eInputMode, EGamepadTextInputLineMode eLineInputMode, const char * pchDescription, uint32_t unCharMax, const char * pchExistingText, bool bUseMinimalMode, uint64_t uUserValue) {
	return base.ShowKeyboard(eInputMode, eLineInputMode, pchDescription, unCharMax, pchExistingText, bUseMinimalMode, uUserValue);
}
EVROverlayError CVROverlay_016::ShowKeyboardForOverlay(VROverlayHandle_t ulOverlayHandle, EGamepadTextInputMode eInputMode, EGamepadTextInputLineMode eLineInputMode, const char * pchDescription, uint32_t unCharMax, const char * pchExistingText, bool bUseMinimalMode, uint64_t uUserValue) {
	return base.ShowKeyboardForOverlay(ulOverlayHandle, eInputMode, eLineInputMode, pchDescription, unCharMax, pchExistingText, bUseMinimalMode, uUserValue);
}
uint32_t CVROverlay_016::GetKeyboardText(VR_OUT_STRING() char * pchText, uint32_t cchText) {
	return base.GetKeyboardText(pchText, cchText);
}
void CVROverlay_016::HideKeyboard() {
	return base.HideKeyboard();
}
void CVROverlay_016::SetKeyboardTransformAbsolute(ETrackingUniverseOrigin eTrackingOrigin, const HmdMatrix34_t * pmatTrackingOriginToKeyboardTransform) {
	return base.SetKeyboardTransformAbsolute(eTrackingOrigin, pmatTrackingOriginToKeyboardTransform);
}
void CVROverlay_016::SetKeyboardPositionForOverlay(VROverlayHandle_t ulOverlayHandle, HmdRect2_t avoidRect) {
	return base.SetKeyboardPositionForOverlay(ulOverlayHandle, avoidRect);
}
EVROverlayError CVROverlay_016::SetOverlayIntersectionMask(VROverlayHandle_t ulOverlayHandle, VROverlayIntersectionMaskPrimitive_t * pMaskPrimitives, uint32_t unNumMaskPrimitives, uint32_t unPrimitiveSize) {
	//return base.SetOverlayIntersectionMask(ulOverlayHandle, pMaskPrimitives, unNumMaskPrimitives, unPrimitiveSize);
	STUBBED();
}
EVROverlayError CVROverlay_016::GetOverlayFlags(VROverlayHandle_t ulOverlayHandle, uint32_t * pFlags) {
	return base.GetOverlayFlags(ulOverlayHandle, pFlags);
}
VRMessageOverlayResponse CVROverlay_016::ShowMessageOverlay(const char* pchText, const char* pchCaption, const char* pchButton0Text, const char* pchButton1Text, const char* pchButton2Text, const char* pchButton3Text) {
	return (VRMessageOverlayResponse) base.ShowMessageOverlay(pchText, pchCaption, pchButton0Text, pchButton1Text, pchButton2Text, pchButton3Text);
}
