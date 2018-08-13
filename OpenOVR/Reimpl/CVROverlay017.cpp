#include "stdafx.h"
#define CVR_IMPL
#include "CVROverlay017.h"
CVR_GEN_IMPL(CVROverlay_017);

using namespace IVROverlay_017;

#include <string>
#define STUBBED() { \
	std::string str = "Hit stubbed file at " __FILE__ " func "  " line " + std::to_string(__LINE__); \
	MessageBoxA(NULL, str.c_str(), "Stubbed func!", MB_OK); \
	throw "stub"; \
}

EVROverlayError CVROverlay_017::FindOverlay(const char * pchOverlayKey, VROverlayHandle_t * pOverlayHandle) {
	return base.FindOverlay(pchOverlayKey, pOverlayHandle);
}
EVROverlayError CVROverlay_017::CreateOverlay(const char * pchOverlayKey, const char * pchOverlayName, VROverlayHandle_t * pOverlayHandle) {
	return base.CreateOverlay(pchOverlayKey, pchOverlayName, pOverlayHandle);
}
EVROverlayError CVROverlay_017::DestroyOverlay(VROverlayHandle_t ulOverlayHandle) {
	return base.DestroyOverlay(ulOverlayHandle);
}
EVROverlayError CVROverlay_017::SetHighQualityOverlay(VROverlayHandle_t ulOverlayHandle) {
	return base.SetHighQualityOverlay(ulOverlayHandle);
}
VROverlayHandle_t CVROverlay_017::GetHighQualityOverlay() {
	return base.GetHighQualityOverlay();
}
uint32_t CVROverlay_017::GetOverlayKey(VROverlayHandle_t ulOverlayHandle, VR_OUT_STRING() char * pchValue, uint32_t unBufferSize, EVROverlayError * pError) {
	return base.GetOverlayKey(ulOverlayHandle, pchValue, unBufferSize, pError);
}
uint32_t CVROverlay_017::GetOverlayName(VROverlayHandle_t ulOverlayHandle, VR_OUT_STRING() char * pchValue, uint32_t unBufferSize, EVROverlayError * pError) {
	return base.GetOverlayName(ulOverlayHandle, pchValue, unBufferSize, pError);
}
EVROverlayError CVROverlay_017::SetOverlayName(VROverlayHandle_t ulOverlayHandle, const char * pchName) {
	return base.SetOverlayName(ulOverlayHandle, pchName);
}
EVROverlayError CVROverlay_017::GetOverlayImageData(VROverlayHandle_t ulOverlayHandle, void * pvBuffer, uint32_t unBufferSize, uint32_t * punWidth, uint32_t * punHeight) {
	return base.GetOverlayImageData(ulOverlayHandle, pvBuffer, unBufferSize, punWidth, punHeight);
}
const char * CVROverlay_017::GetOverlayErrorNameFromEnum(EVROverlayError error) {
	return base.GetOverlayErrorNameFromEnum(error);
}
EVROverlayError CVROverlay_017::SetOverlayRenderingPid(VROverlayHandle_t ulOverlayHandle, uint32_t unPID) {
	return base.SetOverlayRenderingPid(ulOverlayHandle, unPID);
}
uint32_t CVROverlay_017::GetOverlayRenderingPid(VROverlayHandle_t ulOverlayHandle) {
	return base.GetOverlayRenderingPid(ulOverlayHandle);
}
EVROverlayError CVROverlay_017::SetOverlayFlag(VROverlayHandle_t ulOverlayHandle, VROverlayFlags eOverlayFlag, bool bEnabled) {
	return base.SetOverlayFlag(ulOverlayHandle, eOverlayFlag, bEnabled);
}
EVROverlayError CVROverlay_017::GetOverlayFlag(VROverlayHandle_t ulOverlayHandle, VROverlayFlags eOverlayFlag, bool * pbEnabled) {
	return base.GetOverlayFlag(ulOverlayHandle, eOverlayFlag, pbEnabled);
}
EVROverlayError CVROverlay_017::SetOverlayColor(VROverlayHandle_t ulOverlayHandle, float fRed, float fGreen, float fBlue) {
	return base.SetOverlayColor(ulOverlayHandle, fRed, fGreen, fBlue);
}
EVROverlayError CVROverlay_017::GetOverlayColor(VROverlayHandle_t ulOverlayHandle, float * pfRed, float * pfGreen, float * pfBlue) {
	return base.GetOverlayColor(ulOverlayHandle, pfRed, pfGreen, pfBlue);
}
EVROverlayError CVROverlay_017::SetOverlayAlpha(VROverlayHandle_t ulOverlayHandle, float fAlpha) {
	return base.SetOverlayAlpha(ulOverlayHandle, fAlpha);
}
EVROverlayError CVROverlay_017::GetOverlayAlpha(VROverlayHandle_t ulOverlayHandle, float * pfAlpha) {
	return base.GetOverlayAlpha(ulOverlayHandle, pfAlpha);
}
EVROverlayError CVROverlay_017::SetOverlayTexelAspect(VROverlayHandle_t ulOverlayHandle, float fTexelAspect) {
	return base.SetOverlayTexelAspect(ulOverlayHandle, fTexelAspect);
}
EVROverlayError CVROverlay_017::GetOverlayTexelAspect(VROverlayHandle_t ulOverlayHandle, float * pfTexelAspect) {
	return base.GetOverlayTexelAspect(ulOverlayHandle, pfTexelAspect);
}
EVROverlayError CVROverlay_017::SetOverlaySortOrder(VROverlayHandle_t ulOverlayHandle, uint32_t unSortOrder) {
	return base.SetOverlaySortOrder(ulOverlayHandle, unSortOrder);
}
EVROverlayError CVROverlay_017::GetOverlaySortOrder(VROverlayHandle_t ulOverlayHandle, uint32_t * punSortOrder) {
	return base.GetOverlaySortOrder(ulOverlayHandle, punSortOrder);
}
EVROverlayError CVROverlay_017::SetOverlayWidthInMeters(VROverlayHandle_t ulOverlayHandle, float fWidthInMeters) {
	return base.SetOverlayWidthInMeters(ulOverlayHandle, fWidthInMeters);
}
EVROverlayError CVROverlay_017::GetOverlayWidthInMeters(VROverlayHandle_t ulOverlayHandle, float * pfWidthInMeters) {
	return base.GetOverlayWidthInMeters(ulOverlayHandle, pfWidthInMeters);
}
EVROverlayError CVROverlay_017::SetOverlayAutoCurveDistanceRangeInMeters(VROverlayHandle_t ulOverlayHandle, float fMinDistanceInMeters, float fMaxDistanceInMeters) {
	return base.SetOverlayAutoCurveDistanceRangeInMeters(ulOverlayHandle, fMinDistanceInMeters, fMaxDistanceInMeters);
}
EVROverlayError CVROverlay_017::GetOverlayAutoCurveDistanceRangeInMeters(VROverlayHandle_t ulOverlayHandle, float * pfMinDistanceInMeters, float * pfMaxDistanceInMeters) {
	return base.GetOverlayAutoCurveDistanceRangeInMeters(ulOverlayHandle, pfMinDistanceInMeters, pfMaxDistanceInMeters);
}
EVROverlayError CVROverlay_017::SetOverlayTextureColorSpace(VROverlayHandle_t ulOverlayHandle, EColorSpace eTextureColorSpace) {
	return base.SetOverlayTextureColorSpace(ulOverlayHandle, eTextureColorSpace);
}
EVROverlayError CVROverlay_017::GetOverlayTextureColorSpace(VROverlayHandle_t ulOverlayHandle, EColorSpace * peTextureColorSpace) {
	return base.GetOverlayTextureColorSpace(ulOverlayHandle, peTextureColorSpace);
}
EVROverlayError CVROverlay_017::SetOverlayTextureBounds(VROverlayHandle_t ulOverlayHandle, const VRTextureBounds_t * pOverlayTextureBounds) {
	return base.SetOverlayTextureBounds(ulOverlayHandle, pOverlayTextureBounds);
}
EVROverlayError CVROverlay_017::GetOverlayTextureBounds(VROverlayHandle_t ulOverlayHandle, VRTextureBounds_t * pOverlayTextureBounds) {
	return base.GetOverlayTextureBounds(ulOverlayHandle, pOverlayTextureBounds);
}
uint32_t CVROverlay_017::GetOverlayRenderModel(VROverlayHandle_t ulOverlayHandle, char * pchValue, uint32_t unBufferSize, HmdColor_t * pColor, EVROverlayError * pError) {
	return base.GetOverlayRenderModel(ulOverlayHandle, pchValue, unBufferSize, pColor, pError);
}
EVROverlayError CVROverlay_017::SetOverlayRenderModel(VROverlayHandle_t ulOverlayHandle, const char * pchRenderModel, const HmdColor_t * pColor) {
	return base.SetOverlayRenderModel(ulOverlayHandle, pchRenderModel, pColor);
}
EVROverlayError CVROverlay_017::GetOverlayTransformType(VROverlayHandle_t ulOverlayHandle, VROverlayTransformType * peTransformType) {
	return base.GetOverlayTransformType(ulOverlayHandle, (int*)peTransformType);
}
EVROverlayError CVROverlay_017::SetOverlayTransformAbsolute(VROverlayHandle_t ulOverlayHandle, ETrackingUniverseOrigin eTrackingOrigin, const HmdMatrix34_t * pmatTrackingOriginToOverlayTransform) {
	return base.SetOverlayTransformAbsolute(ulOverlayHandle, eTrackingOrigin, pmatTrackingOriginToOverlayTransform);
}
EVROverlayError CVROverlay_017::GetOverlayTransformAbsolute(VROverlayHandle_t ulOverlayHandle, ETrackingUniverseOrigin * peTrackingOrigin, HmdMatrix34_t * pmatTrackingOriginToOverlayTransform) {
	return base.GetOverlayTransformAbsolute(ulOverlayHandle, peTrackingOrigin, pmatTrackingOriginToOverlayTransform);
}
EVROverlayError CVROverlay_017::SetOverlayTransformTrackedDeviceRelative(VROverlayHandle_t ulOverlayHandle, TrackedDeviceIndex_t unTrackedDevice, const HmdMatrix34_t * pmatTrackedDeviceToOverlayTransform) {
	return base.SetOverlayTransformTrackedDeviceRelative(ulOverlayHandle, unTrackedDevice, pmatTrackedDeviceToOverlayTransform);
}
EVROverlayError CVROverlay_017::GetOverlayTransformTrackedDeviceRelative(VROverlayHandle_t ulOverlayHandle, TrackedDeviceIndex_t * punTrackedDevice, HmdMatrix34_t * pmatTrackedDeviceToOverlayTransform) {
	return base.GetOverlayTransformTrackedDeviceRelative(ulOverlayHandle, punTrackedDevice, pmatTrackedDeviceToOverlayTransform);
}
EVROverlayError CVROverlay_017::SetOverlayTransformTrackedDeviceComponent(VROverlayHandle_t ulOverlayHandle, TrackedDeviceIndex_t unDeviceIndex, const char * pchComponentName) {
	return base.SetOverlayTransformTrackedDeviceComponent(ulOverlayHandle, unDeviceIndex, pchComponentName);
}
EVROverlayError CVROverlay_017::GetOverlayTransformTrackedDeviceComponent(VROverlayHandle_t ulOverlayHandle, TrackedDeviceIndex_t * punDeviceIndex, char * pchComponentName, uint32_t unComponentNameSize) {
	return base.GetOverlayTransformTrackedDeviceComponent(ulOverlayHandle, punDeviceIndex, pchComponentName, unComponentNameSize);
}
EVROverlayError CVROverlay_017::GetOverlayTransformOverlayRelative(VROverlayHandle_t ulOverlayHandle, VROverlayHandle_t * ulOverlayHandleParent, HmdMatrix34_t * pmatParentOverlayToOverlayTransform) {
	return base.GetOverlayTransformOverlayRelative(ulOverlayHandle, ulOverlayHandleParent, pmatParentOverlayToOverlayTransform);
}
EVROverlayError CVROverlay_017::SetOverlayTransformOverlayRelative(VROverlayHandle_t ulOverlayHandle, VROverlayHandle_t ulOverlayHandleParent, const HmdMatrix34_t * pmatParentOverlayToOverlayTransform) {
	return base.SetOverlayTransformOverlayRelative(ulOverlayHandle, ulOverlayHandleParent, pmatParentOverlayToOverlayTransform);
}
EVROverlayError CVROverlay_017::ShowOverlay(VROverlayHandle_t ulOverlayHandle) {
	return base.ShowOverlay(ulOverlayHandle);
}
EVROverlayError CVROverlay_017::HideOverlay(VROverlayHandle_t ulOverlayHandle) {
	return base.HideOverlay(ulOverlayHandle);
}
bool CVROverlay_017::IsOverlayVisible(VROverlayHandle_t ulOverlayHandle) {
	return base.IsOverlayVisible(ulOverlayHandle);
}
EVROverlayError CVROverlay_017::GetTransformForOverlayCoordinates(VROverlayHandle_t ulOverlayHandle, ETrackingUniverseOrigin eTrackingOrigin, HmdVector2_t coordinatesInOverlay, HmdMatrix34_t * pmatTransform) {
	return base.GetTransformForOverlayCoordinates(ulOverlayHandle, eTrackingOrigin, coordinatesInOverlay, pmatTransform);
}
bool CVROverlay_017::PollNextOverlayEvent(VROverlayHandle_t ulOverlayHandle, VREvent_t * pEvent, uint32_t uncbVREvent) {
	return base.PollNextOverlayEvent(ulOverlayHandle, pEvent, uncbVREvent);
}
EVROverlayError CVROverlay_017::GetOverlayInputMethod(VROverlayHandle_t ulOverlayHandle, VROverlayInputMethod * peInputMethod) {
	return base.GetOverlayInputMethod(ulOverlayHandle, (int*)peInputMethod);
}
EVROverlayError CVROverlay_017::SetOverlayInputMethod(VROverlayHandle_t ulOverlayHandle, VROverlayInputMethod eInputMethod) {
	return base.SetOverlayInputMethod(ulOverlayHandle, eInputMethod);
}
EVROverlayError CVROverlay_017::GetOverlayMouseScale(VROverlayHandle_t ulOverlayHandle, HmdVector2_t * pvecMouseScale) {
	return base.GetOverlayMouseScale(ulOverlayHandle, pvecMouseScale);
}
EVROverlayError CVROverlay_017::SetOverlayMouseScale(VROverlayHandle_t ulOverlayHandle, const HmdVector2_t * pvecMouseScale) {
	return base.SetOverlayMouseScale(ulOverlayHandle, pvecMouseScale);
}
bool CVROverlay_017::ComputeOverlayIntersection(VROverlayHandle_t ulOverlayHandle, const VROverlayIntersectionParams_t * pParams, VROverlayIntersectionResults_t * pResults) {
	//return base.ComputeOverlayIntersection(ulOverlayHandle, pParams, pResults);
	STUBBED();
}
bool CVROverlay_017::HandleControllerOverlayInteractionAsMouse(VROverlayHandle_t ulOverlayHandle, TrackedDeviceIndex_t unControllerDeviceIndex) {
	return base.HandleControllerOverlayInteractionAsMouse(ulOverlayHandle, unControllerDeviceIndex);
}
bool CVROverlay_017::IsHoverTargetOverlay(VROverlayHandle_t ulOverlayHandle) {
	return base.IsHoverTargetOverlay(ulOverlayHandle);
}
VROverlayHandle_t CVROverlay_017::GetGamepadFocusOverlay() {
	return base.GetGamepadFocusOverlay();
}
EVROverlayError CVROverlay_017::SetGamepadFocusOverlay(VROverlayHandle_t ulNewFocusOverlay) {
	return base.SetGamepadFocusOverlay(ulNewFocusOverlay);
}
EVROverlayError CVROverlay_017::SetOverlayNeighbor(EOverlayDirection eDirection, VROverlayHandle_t ulFrom, VROverlayHandle_t ulTo) {
	return base.SetOverlayNeighbor(eDirection, ulFrom, ulTo);
}
EVROverlayError CVROverlay_017::MoveGamepadFocusToNeighbor(EOverlayDirection eDirection, VROverlayHandle_t ulFrom) {
	return base.MoveGamepadFocusToNeighbor(eDirection, ulFrom);
}
EVROverlayError CVROverlay_017::SetOverlayDualAnalogTransform(VROverlayHandle_t ulOverlay, EDualAnalogWhich eWhich, const HmdVector2_t & vCenter, float fRadius) {
	return base.SetOverlayDualAnalogTransform(ulOverlay, eWhich, vCenter, fRadius);
}
EVROverlayError CVROverlay_017::GetOverlayDualAnalogTransform(VROverlayHandle_t ulOverlay, EDualAnalogWhich eWhich, HmdVector2_t * pvCenter, float * pfRadius) {
	return base.GetOverlayDualAnalogTransform(ulOverlay, eWhich, pvCenter, pfRadius);
}
EVROverlayError CVROverlay_017::SetOverlayTexture(VROverlayHandle_t ulOverlayHandle, const Texture_t * pTexture) {
	return base.SetOverlayTexture(ulOverlayHandle, pTexture);
}
EVROverlayError CVROverlay_017::ClearOverlayTexture(VROverlayHandle_t ulOverlayHandle) {
	return base.ClearOverlayTexture(ulOverlayHandle);
}
EVROverlayError CVROverlay_017::SetOverlayRaw(VROverlayHandle_t ulOverlayHandle, void * pvBuffer, uint32_t unWidth, uint32_t unHeight, uint32_t unDepth) {
	return base.SetOverlayRaw(ulOverlayHandle, pvBuffer, unWidth, unHeight, unDepth);
}
EVROverlayError CVROverlay_017::SetOverlayFromFile(VROverlayHandle_t ulOverlayHandle, const char * pchFilePath) {
	return base.SetOverlayFromFile(ulOverlayHandle, pchFilePath);
}
EVROverlayError CVROverlay_017::GetOverlayTexture(VROverlayHandle_t ulOverlayHandle, void ** pNativeTextureHandle, void * pNativeTextureRef, uint32_t * pWidth, uint32_t * pHeight, uint32_t * pNativeFormat, ETextureType * pAPIType, EColorSpace * pColorSpace, VRTextureBounds_t * pTextureBounds) {
	return base.GetOverlayTexture(ulOverlayHandle, pNativeTextureHandle, pNativeTextureRef, pWidth, pHeight, pNativeFormat, pAPIType, pColorSpace, pTextureBounds);
}
EVROverlayError CVROverlay_017::ReleaseNativeOverlayHandle(VROverlayHandle_t ulOverlayHandle, void * pNativeTextureHandle) {
	return base.ReleaseNativeOverlayHandle(ulOverlayHandle, pNativeTextureHandle);
}
EVROverlayError CVROverlay_017::GetOverlayTextureSize(VROverlayHandle_t ulOverlayHandle, uint32_t * pWidth, uint32_t * pHeight) {
	return base.GetOverlayTextureSize(ulOverlayHandle, pWidth, pHeight);
}
EVROverlayError CVROverlay_017::CreateDashboardOverlay(const char * pchOverlayKey, const char * pchOverlayFriendlyName, VROverlayHandle_t * pMainHandle, VROverlayHandle_t * pThumbnailHandle) {
	return base.CreateDashboardOverlay(pchOverlayKey, pchOverlayFriendlyName, pMainHandle, pThumbnailHandle);
}
bool CVROverlay_017::IsDashboardVisible() {
	return base.IsDashboardVisible();
}
bool CVROverlay_017::IsActiveDashboardOverlay(VROverlayHandle_t ulOverlayHandle) {
	return base.IsActiveDashboardOverlay(ulOverlayHandle);
}
EVROverlayError CVROverlay_017::SetDashboardOverlaySceneProcess(VROverlayHandle_t ulOverlayHandle, uint32_t unProcessId) {
	return base.SetDashboardOverlaySceneProcess(ulOverlayHandle, unProcessId);
}
EVROverlayError CVROverlay_017::GetDashboardOverlaySceneProcess(VROverlayHandle_t ulOverlayHandle, uint32_t * punProcessId) {
	return base.GetDashboardOverlaySceneProcess(ulOverlayHandle, punProcessId);
}
void CVROverlay_017::ShowDashboard(const char * pchOverlayToShow) {
	return base.ShowDashboard(pchOverlayToShow);
}
TrackedDeviceIndex_t CVROverlay_017::GetPrimaryDashboardDevice() {
	return base.GetPrimaryDashboardDevice();
}
EVROverlayError CVROverlay_017::ShowKeyboard(EGamepadTextInputMode eInputMode, EGamepadTextInputLineMode eLineInputMode, const char * pchDescription, uint32_t unCharMax, const char * pchExistingText, bool bUseMinimalMode, uint64_t uUserValue) {
	return base.ShowKeyboard(eInputMode, eLineInputMode, pchDescription, unCharMax, pchExistingText, bUseMinimalMode, uUserValue);
}
EVROverlayError CVROverlay_017::ShowKeyboardForOverlay(VROverlayHandle_t ulOverlayHandle, EGamepadTextInputMode eInputMode, EGamepadTextInputLineMode eLineInputMode, const char * pchDescription, uint32_t unCharMax, const char * pchExistingText, bool bUseMinimalMode, uint64_t uUserValue) {
	return base.ShowKeyboardForOverlay(ulOverlayHandle, eInputMode, eLineInputMode, pchDescription, unCharMax, pchExistingText, bUseMinimalMode, uUserValue);
}
uint32_t CVROverlay_017::GetKeyboardText(VR_OUT_STRING() char * pchText, uint32_t cchText) {
	return base.GetKeyboardText(pchText, cchText);
}
void CVROverlay_017::HideKeyboard() {
	return base.HideKeyboard();
}
void CVROverlay_017::SetKeyboardTransformAbsolute(ETrackingUniverseOrigin eTrackingOrigin, const HmdMatrix34_t * pmatTrackingOriginToKeyboardTransform) {
	return base.SetKeyboardTransformAbsolute(eTrackingOrigin, pmatTrackingOriginToKeyboardTransform);
}
void CVROverlay_017::SetKeyboardPositionForOverlay(VROverlayHandle_t ulOverlayHandle, HmdRect2_t avoidRect) {
	return base.SetKeyboardPositionForOverlay(ulOverlayHandle, avoidRect);
}
EVROverlayError CVROverlay_017::SetOverlayIntersectionMask(VROverlayHandle_t ulOverlayHandle, VROverlayIntersectionMaskPrimitive_t * pMaskPrimitives, uint32_t unNumMaskPrimitives, uint32_t unPrimitiveSize) {
	//return base.SetOverlayIntersectionMask(ulOverlayHandle, pMaskPrimitives, unNumMaskPrimitives, unPrimitiveSize);
	STUBBED();
}
EVROverlayError CVROverlay_017::GetOverlayFlags(VROverlayHandle_t ulOverlayHandle, uint32_t * pFlags) {
	return base.GetOverlayFlags(ulOverlayHandle, pFlags);
}
VRMessageOverlayResponse CVROverlay_017::ShowMessageOverlay(const char* pchText, const char* pchCaption, const char* pchButton0Text, const char* pchButton1Text, const char* pchButton2Text, const char* pchButton3Text) {
	return (VRMessageOverlayResponse) base.ShowMessageOverlay(pchText, pchCaption, pchButton0Text, pchButton1Text, pchButton2Text, pchButton3Text);
}
void CVROverlay_017::CloseMessageOverlay() {
	return base.CloseMessageOverlay();
}
