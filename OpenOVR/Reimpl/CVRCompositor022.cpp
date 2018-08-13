#include "stdafx.h"
#define CVR_IMPL
#include "CVRCompositor022.h"
CVR_GEN_IMPL(CVRCompositor_022);

using namespace vr;
using namespace IVRCompositor_022;

#define SIMPLE_REDIR(rettype, name) \
rettype CVRCompositor_022::name() { \
	return base.name(); \
}

#define ADV_GETSET_REDIR(type, name, getter, setter) \
SIMPLE_REDIR(type, getter ## name) \
void CVRCompositor_022::setter ## name(type val) { \
	return base.setter ## name(val); \
}

#define GETSET_REDIR(type, name) ADV_GETSET_REDIR(type, name, Get, Set)

SIMPLE_REDIR(float, GetCurrentGridAlpha);
SIMPLE_REDIR(void, ClearSkyboxOverride);
SIMPLE_REDIR(void, CompositorBringToFront);
SIMPLE_REDIR(void, CompositorGoToBack);
SIMPLE_REDIR(void, CompositorQuit);
SIMPLE_REDIR(bool, IsFullscreen);
SIMPLE_REDIR(uint32_t, GetCurrentSceneFocusProcess);
SIMPLE_REDIR(uint32_t, GetLastFrameRenderer);
SIMPLE_REDIR(bool, CanRenderScene);
SIMPLE_REDIR(bool, IsMirrorWindowVisible);
SIMPLE_REDIR(void, CompositorDumpImages);
SIMPLE_REDIR(bool, ShouldAppRenderWithLowResources);
SIMPLE_REDIR(void, ForceReconnectProcess);
SIMPLE_REDIR(float, GetFrameTimeRemaining);
SIMPLE_REDIR(void, ShowMirrorWindow);
SIMPLE_REDIR(void, HideMirrorWindow);

GETSET_REDIR(ETrackingUniverseOrigin, TrackingSpace);

#undef SIMPLE_REDIR
#undef ADV_GETSET_REDIR
#undef GETSET_REDIR

CVRCompositor_022::CVRCompositor_022() {
	// Handled in base
}

CVRCompositor_022::~CVRCompositor_022() {
	// Handled in base
}

EVRCompositorError CVRCompositor_022::WaitGetPoses(VR_ARRAY_COUNT(renderPoseArrayCount)TrackedDevicePose_t * renderPoseArray, uint32_t renderPoseArrayCount,
	VR_ARRAY_COUNT(gamePoseArrayCount)TrackedDevicePose_t * gamePoseArray, uint32_t gamePoseArrayCount) {

	return (EVRCompositorError)base.WaitGetPoses(renderPoseArray, renderPoseArrayCount, gamePoseArray, gamePoseArrayCount);
}

EVRCompositorError CVRCompositor_022::GetLastPoses(VR_ARRAY_COUNT(renderPoseArrayCount)TrackedDevicePose_t * renderPoseArray, uint32_t renderPoseArrayCount,
	VR_ARRAY_COUNT(gamePoseArrayCount)TrackedDevicePose_t * gamePoseArray, uint32_t gamePoseArrayCount) {

	return (EVRCompositorError)base.GetLastPoses(renderPoseArray, renderPoseArrayCount, gamePoseArray, gamePoseArrayCount);
}

EVRCompositorError CVRCompositor_022::GetLastPoseForTrackedDeviceIndex(TrackedDeviceIndex_t unDeviceIndex, TrackedDevicePose_t * pOutputPose,
	TrackedDevicePose_t * pOutputGamePose) {

	return (EVRCompositorError)base.GetLastPoseForTrackedDeviceIndex(unDeviceIndex, pOutputPose, pOutputGamePose);
}

EVRCompositorError CVRCompositor_022::Submit(EVREye eye, const Texture_t * texture, const VRTextureBounds_t * bounds, EVRSubmitFlags submitFlags) {

	return (EVRCompositorError)base.Submit(eye, texture, bounds, submitFlags);
}

void CVRCompositor_022::ClearLastSubmittedFrame() {
	return base.ClearLastSubmittedFrame();
}

void CVRCompositor_022::PostPresentHandoff() {
	return base.PostPresentHandoff();
}

bool CVRCompositor_022::GetFrameTiming(Compositor_FrameTiming * pTiming, uint32_t unFramesAgo) {
	OOVR_Compositor_FrameTiming dest;
	bool result = base.GetFrameTiming(&dest, unFramesAgo);
	// Copy just the relevant data in
	memcpy(pTiming, &dest, sizeof(Compositor_FrameTiming));

	// Make sure we know if the struct changes
	// TODO enable C++17
	// static_assert(sizeof(Compositor_FrameTiming) == sizeof(OOVR_Compositor_FrameTiming));

	return result;
}

uint32_t CVRCompositor_022::GetFrameTimings(Compositor_FrameTiming * pTiming, uint32_t nFrames) {
	OOVR_Compositor_FrameTiming dest;
	bool result = base.GetFrameTimings(&dest, nFrames);
	// Copy just the relevant data in
	memcpy(pTiming, &dest, sizeof(Compositor_FrameTiming));

	// Make sure we know if the struct changes
	// TODO enable C++17
	// static_assert(sizeof(Compositor_FrameTiming) == sizeof(OOVR_Compositor_FrameTiming));

	return result;
}

void CVRCompositor_022::GetCumulativeStats(Compositor_CumulativeStats * pStats, uint32_t nStatsSizeInBytes) {
	throw "stub";
	// TODO sort out structs
	//return base.GetCumulativeStats(pStats, nStatsSizeInBytes);
}

void CVRCompositor_022::FadeToColor(float fSeconds, float fRed, float fGreen, float fBlue, float fAlpha, bool bBackground) {
	return base.FadeToColor(fSeconds, fRed, fGreen, fBlue, fAlpha, bBackground);
}

HmdColor_t CVRCompositor_022::GetCurrentFadeColor(bool bBackground) {
	return base.GetCurrentFadeColor(bBackground);
}

void CVRCompositor_022::FadeGrid(float fSeconds, bool bFadeIn) {
	return base.FadeGrid(fSeconds, bFadeIn);
}

EVRCompositorError CVRCompositor_022::SetSkyboxOverride(VR_ARRAY_COUNT(unTextureCount) const Texture_t * pTextures, uint32_t unTextureCount) {
	return (EVRCompositorError)base.SetSkyboxOverride(pTextures, unTextureCount);
}

void CVRCompositor_022::ForceInterleavedReprojectionOn(bool bOverride) {
	return base.ForceInterleavedReprojectionOn(bOverride);
}

void CVRCompositor_022::SuspendRendering(bool bSuspend) {
	return base.SuspendRendering(bSuspend);
}

EVRCompositorError CVRCompositor_022::GetMirrorTextureD3D11(EVREye eEye, void * pD3D11DeviceOrResource, void ** ppD3D11ShaderResourceView) {
	return (EVRCompositorError)base.GetMirrorTextureD3D11(eEye, pD3D11DeviceOrResource, ppD3D11ShaderResourceView);
}

void CVRCompositor_022::ReleaseMirrorTextureD3D11(void * pD3D11ShaderResourceView) {
	return base.ReleaseMirrorTextureD3D11(pD3D11ShaderResourceView);
}

EVRCompositorError CVRCompositor_022::GetMirrorTextureGL(EVREye eEye, glUInt_t * pglTextureId, glSharedTextureHandle_t * pglSharedTextureHandle) {
	return (EVRCompositorError)base.GetMirrorTextureGL(eEye, pglTextureId, pglSharedTextureHandle);
}

bool CVRCompositor_022::ReleaseSharedGLTexture(glUInt_t glTextureId, glSharedTextureHandle_t glSharedTextureHandle) {
	return base.ReleaseSharedGLTexture(glTextureId, glSharedTextureHandle);
}

void CVRCompositor_022::LockGLSharedTextureForAccess(glSharedTextureHandle_t glSharedTextureHandle) {
	return base.LockGLSharedTextureForAccess(glSharedTextureHandle);
}

void CVRCompositor_022::UnlockGLSharedTextureForAccess(glSharedTextureHandle_t glSharedTextureHandle) {
	return base.UnlockGLSharedTextureForAccess(glSharedTextureHandle);
}

uint32_t CVRCompositor_022::GetVulkanInstanceExtensionsRequired(VR_OUT_STRING() char * pchValue, uint32_t unBufferSize) {
	return base.GetVulkanInstanceExtensionsRequired(pchValue, unBufferSize);
}

uint32_t CVRCompositor_022::GetVulkanDeviceExtensionsRequired(VkPhysicalDevice_T * pPhysicalDevice, VR_OUT_STRING() char * pchValue, uint32_t unBufferSize) {
	return base.GetVulkanDeviceExtensionsRequired(pPhysicalDevice, pchValue, unBufferSize);
}

void CVRCompositor_022::SetExplicitTimingMode(EVRCompositorTimingMode eTimingMode) {
	return base.SetExplicitTimingMode(eTimingMode);
}

EVRCompositorError CVRCompositor_022::SubmitExplicitTimingData() {
	return (EVRCompositorError)base.SubmitExplicitTimingData();
}
