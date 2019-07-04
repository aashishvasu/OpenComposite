#include "stdafx.h"
#define BASE_IMPL

#include "Misc/Config.h"

#include "OVR_CAPI.h"
#include "libovr_wrapper.h"
#include "convert.h"

#include "Extras/OVR_Math.h"
using namespace OVR;

using namespace std;

#include "BaseCompositor.h"
#include "BaseOverlay.h"

// For the left and right hand constants - TODO move them to their own file
#include "BaseSystem.h"
#include "static_bases.gen.h"

// Need the LibOVR Vulkan headers for the GetVulkan[Device|Instance]ExtensionsRequired methods
#if defined(SUPPORT_VK)
#include "OVR_CAPI_Vk.h"
#endif
#if defined(SUPPORT_DX)
#include "OVR_CAPI_D3D.h"
#endif

#include "Misc/ScopeGuard.h"
#include "Drivers/Backend.h"

using namespace vr;
using namespace IVRCompositor_022;

typedef int ovr_enum_t;

#define SESS (*ovr::session)

BaseCompositor::BaseCompositor() {
}

BaseCompositor::~BaseCompositor() {
}

void BaseCompositor::SetTrackingSpace(ETrackingUniverseOrigin eOrigin) {
	ovrTrackingOrigin origin = ovrTrackingOrigin_FloorLevel;
	if (eOrigin == TrackingUniverseSeated) {
		origin = ovrTrackingOrigin_EyeLevel;
	}

	OOVR_FAILED_OVR_ABORT(ovr_SetTrackingOriginType(SESS, origin));
}

ETrackingUniverseOrigin BaseCompositor::GetTrackingSpace() {
	if (ovr_GetTrackingOriginType(SESS) == ovrTrackingOrigin_EyeLevel) {
		return TrackingUniverseSeated;
	}
	else {
		return TrackingUniverseStanding;
	}
}

ovr_enum_t BaseCompositor::WaitGetPoses(TrackedDevicePose_t * renderPoseArray, uint32_t renderPoseArrayCount,
	TrackedDevicePose_t * gamePoseArray, uint32_t gamePoseArrayCount) {

	// Assume this method isn't being called between frames, b/c it really shouldn't be.
	leftEyeSubmitted = false;
	rightEyeSubmitted = false;

	BackendManager::Instance().WaitForTrackingData();

	return GetLastPoses(renderPoseArray, renderPoseArrayCount, gamePoseArray, gamePoseArrayCount);
}

void BaseCompositor::GetSinglePoseRendering(ETrackingUniverseOrigin origin, TrackedDeviceIndex_t unDeviceIndex, TrackedDevicePose_t * pOutputPose) {
	BackendManager::Instance().GetSinglePose(origin, unDeviceIndex, pOutputPose, ETrackingStateType::TrackingStateType_Rendering);
}

Matrix4f BaseCompositor::GetHandTransform() {
	float deg_to_rad = math_pi / 180;

	// The angle offset between the Touch and Vive controllers.
	// If this is incorrect, virtual hands will feel off.
	float controller_offset_angle = 39.5;

	// When testing to try and find the correct value above, uncomment
	//  this to lock the controller perfectly flat.
	// ovrPose.ThePose.Orientation = { 0,0,0,1 };

	Vector3f rotateAxis = Vector3f(1, 0, 0);
	Quatf rotation = Quatf(rotateAxis, controller_offset_angle * deg_to_rad); //count++ * 0.01f);

	Matrix4f transform(rotation);

	// Controller offset
	// Note this is about right, found by playing around in Unity until everything
	//  roughly lines up. If you want to contribute better numbers, please go ahead!
	transform.SetTranslation(Vector3f(0.0f, 0.0353f, -0.0451f));

	return transform;
}

ovr_enum_t BaseCompositor::GetLastPoses(TrackedDevicePose_t * renderPoseArray, uint32_t renderPoseArrayCount,
	TrackedDevicePose_t * gamePoseArray, uint32_t gamePoseArrayCount) {

	ETrackingUniverseOrigin origin = GetUnsafeBaseSystem()->_GetTrackingOrigin();

	for (uint32_t i = 0; i < max(gamePoseArrayCount, renderPoseArrayCount); i++) {
		TrackedDevicePose_t *renderPose = i < renderPoseArrayCount ? renderPoseArray + i : NULL;
		TrackedDevicePose_t *gamePose = i < gamePoseArrayCount ? gamePoseArray + i : NULL;

		if (renderPose) {
			GetSinglePoseRendering(origin, i, renderPose);
		}

		if (gamePose) {
			if (renderPose) {
				*gamePose = *renderPose;
			}
			else {
				GetSinglePoseRendering(origin, i, gamePose);
			}
		}
	}

	return VRCompositorError_None;
}

ovr_enum_t BaseCompositor::GetLastPoseForTrackedDeviceIndex(TrackedDeviceIndex_t unDeviceIndex, TrackedDevicePose_t * pOutputPose,
	TrackedDevicePose_t * pOutputGamePose) {

	if (unDeviceIndex < 0 || unDeviceIndex >= k_unMaxTrackedDeviceCount) {
		return VRCompositorError_IndexOutOfRange;
	}

	ETrackingUniverseOrigin origin = GetUnsafeBaseSystem()->_GetTrackingOrigin();

	TrackedDevicePose_t pose;
	GetSinglePoseRendering(origin, unDeviceIndex, &pose);

	if (pOutputPose) {
		*pOutputPose = pose;
	}

	if (pOutputGamePose) {
		*pOutputGamePose = pose;
	}

	return VRCompositorError_None;
}

DX11Compositor *BaseCompositor::dxcomp;

Compositor* BaseCompositor::CreateCompositorAPI(const vr::Texture_t* texture, const OVR::Sizei& fovTextureSize)
{
	Compositor* comp = nullptr;

	switch (texture->eType) {
#ifdef SUPPORT_GL
	case TextureType_OpenGL: {
		comp = new GLCompositor(fovTextureSize);
		break;
	}
#endif
#ifdef SUPPORT_DX
	case TextureType_DirectX: {
		if (!oovr_global_configuration.DX10Mode())
			comp = new DX11Compositor((ID3D11Texture2D*)texture->handle);
		else
			comp = new DX10Compositor((ID3D10Texture2D*)texture->handle);

		dxcomp = (DX11Compositor*) comp;

		break;
	}
#endif
#if defined(SUPPORT_VK)
	case TextureType_Vulkan: {
		comp = new VkCompositor(texture);
		break;
	}
#endif
#if defined(SUPPORT_DX12)
	case TextureType_DirectX12: {
		compositor = new DX12Compositor((D3D12TextureData_t*)texture->handle, fovTextureSize, chains);
		break;
	}
#endif
	default:
		string err = "[BaseCompositor::Submit] Unsupported texture type: " + to_string(texture->eType);
		OOVR_ABORT(err.c_str());
	}

	if (comp->GetSwapChain() == NULL && texture->eType != TextureType_DirectX && texture->eType != TextureType_Vulkan) {
		OOVR_ABORT("Failed to create texture.");
	}

	return comp;
}

ovr_enum_t BaseCompositor::Submit(EVREye eye, const Texture_t * texture, const VRTextureBounds_t * bounds, EVRSubmitFlags submitFlags) {
	bool isFirstEye = !leftEyeSubmitted && !rightEyeSubmitted;

	bool eyeState = false;
	if (eye == Eye_Left)
		eyeState = leftEyeSubmitted;
	else
		eyeState = rightEyeSubmitted;

	if (eyeState) {
		OOVR_ABORT("Eye already submitted!");
	}

	if (eye == Eye_Left)
		leftEyeSubmitted = true;
	else
		rightEyeSubmitted = true;

	// Handle null textures
	// Rather surprisingly, it's perfectly valid to pass null textures to SteamVR. So far, I've
	// only seen this used in Sparc (see #19). If a game tries to do this, ensure that either neither or
	// both eyes do so, and don't actually send a frame to LibOVR.
	bool textureNull = texture->handle == nullptr;
	if (isFirstEye) {
		isNullRender = textureNull;
	} else if(textureNull != isNullRender) {
		OOVR_ABORT("Cannot mismatch first and second eye renders");
	}

	if(!textureNull)
		BackendManager::Instance().StoreEyeTexture(eye, texture, bounds, submitFlags, isFirstEye);

	if (leftEyeSubmitted && rightEyeSubmitted) {
		if(!isNullRender)
			BackendManager::Instance().SubmitFrames(isInSkybox);

		leftEyeSubmitted = false;
		rightEyeSubmitted = false;
	}

	return VRCompositorError_None;
}

void BaseCompositor::ClearLastSubmittedFrame() {
	STUBBED();
}

void BaseCompositor::PostPresentHandoff() {
	// It appears (from the documentation) that SteamVR will, even after all frames are submitted, not begin
	//  compositing the submitted textures until WaitGetPoses is called. Thus is you want to do some rendering
	//  or game logic or whatever, it will delay the compositor. Calling this tells SteamVR that no further changes
	//  are to be made to the frame, and it can begin the compositor - in the aforementioned cases, this would be
	//  called directly after the last Submit call.
	//
	// On the other hand, LibOVR starts compositing as soon as ovr_EndFrame is called. So we don't have to do
	//  anything here.
	//
	// TODO: use ovr_EndFrame and co instead of ovr_SubmitFrame for better performance, not just here but in all cases
	//  that way we can call ovr_WaitToBeginFrame in WaitGetPoses to mimick SteamVR.
}

bool BaseCompositor::GetFrameTiming(OOVR_Compositor_FrameTiming * pTiming, uint32_t unFramesAgo) {
	return BackendManager::Instance().GetFrameTiming(pTiming, unFramesAgo);
}

uint32_t BaseCompositor::GetFrameTimings(OOVR_Compositor_FrameTiming * pTiming, uint32_t nFrames) {
	STUBBED();
}

float BaseCompositor::GetFrameTimeRemaining() {
	STUBBED();
}

void BaseCompositor::GetCumulativeStats(OOVR_Compositor_CumulativeStats * pStats, uint32_t nStatsSizeInBytes) {
	STUBBED();
}

void BaseCompositor::FadeToColor(float fSeconds, float fRed, float fGreen, float fBlue, float fAlpha, bool bBackground) {
	fadeTime = fSeconds;
	fadeColour.r = fRed;
	fadeColour.g = fGreen;
	fadeColour.b = fBlue;
	fadeColour.a = fAlpha;

	// TODO what does background do?
}

HmdColor_t BaseCompositor::GetCurrentFadeColor(bool bBackground) {
	return fadeColour;
}

void BaseCompositor::FadeGrid(float fSeconds, bool bFadeIn) {
	// This is the app telling SteamVR to fade from the rendered scene into the skybox, eg before the
	//  app loads a new level (this is how the default SteamVR Unity plugin works).
	//
	// Let's not bother implementing the fade (that would be a LOT of work), and just skip straight over.
	isInSkybox = bFadeIn;

	// TODO suppress input while in this mode
}

float BaseCompositor::GetCurrentGridAlpha() {
	STUBBED();
}

ovr_enum_t BaseCompositor::SetSkyboxOverride(const Texture_t * pTextures, uint32_t unTextureCount) {
	return BackendManager::Instance().SetSkyboxOverride(pTextures, unTextureCount);
}

void BaseCompositor::ClearSkyboxOverride() {
	BackendManager::Instance().ClearSkyboxOverride();
}

void BaseCompositor::CompositorBringToFront() {
	// No actions required, Oculus runs via direct mode
}

void BaseCompositor::CompositorGoToBack() {
	STUBBED();
}

void BaseCompositor::CompositorQuit() {
	STUBBED();
}

bool BaseCompositor::IsFullscreen() {
	STUBBED();
}

uint32_t BaseCompositor::GetCurrentSceneFocusProcess() {
	STUBBED();
}

uint32_t BaseCompositor::GetLastFrameRenderer() {
	STUBBED();
}

bool BaseCompositor::CanRenderScene() {
	return true; // TODO implement
}

void BaseCompositor::ShowMirrorWindow() {
	STUBBED();
}

void BaseCompositor::HideMirrorWindow() {
	STUBBED();
}

bool BaseCompositor::IsMirrorWindowVisible() {
	STUBBED();
}

void BaseCompositor::CompositorDumpImages() {
	STUBBED();
}

bool BaseCompositor::ShouldAppRenderWithLowResources() {
	// TODO put in config file
	return false;
}

void BaseCompositor::ForceInterleavedReprojectionOn(bool bOverride) {
	// Force timewarp on? Yeah right.
}

void BaseCompositor::ForceReconnectProcess() {
	// We should always be connected
}

void BaseCompositor::SuspendRendering(bool bSuspend) {
	// TODO
	// I'm not sure what the purpose of this function is. If you know, please tell me.
	// - ZNix
	//STUBBED();
}

#if defined(SUPPORT_DX)
ovr_enum_t BaseCompositor::GetMirrorTextureD3D11(EVREye eEye, void * pD3D11DeviceOrResource, void ** ppD3D11ShaderResourceView) {
	return BackendManager::Instance().GetMirrorTextureD3D11(eEye, pD3D11DeviceOrResource, ppD3D11ShaderResourceView);
}
#else
ovr_enum_t BaseCompositor::GetMirrorTextureD3D11(EVREye eEye, void * pD3D11DeviceOrResource, void ** ppD3D11ShaderResourceView) {
	OOVR_ABORT("Cannot get D3D mirror texture - D3D support disabled");
}
#endif

void BaseCompositor::ReleaseMirrorTextureD3D11(void * pD3D11ShaderResourceView) {
	return BackendManager::Instance().ReleaseMirrorTextureD3D11(pD3D11ShaderResourceView);
}

ovr_enum_t BaseCompositor::GetMirrorTextureGL(EVREye eEye, glUInt_t * pglTextureId, glSharedTextureHandle_t * pglSharedTextureHandle) {
	STUBBED();
}

bool BaseCompositor::ReleaseSharedGLTexture(glUInt_t glTextureId, glSharedTextureHandle_t glSharedTextureHandle) {
	STUBBED();
}

void BaseCompositor::LockGLSharedTextureForAccess(glSharedTextureHandle_t glSharedTextureHandle) {
	STUBBED();
}

void BaseCompositor::UnlockGLSharedTextureForAccess(glSharedTextureHandle_t glSharedTextureHandle) {
	STUBBED();
}

uint32_t BaseCompositor::GetVulkanInstanceExtensionsRequired(VR_OUT_STRING() char * pchValue, uint32_t unBufferSize) {
#if defined(SUPPORT_VK)
	// Whaddya know, the Oculus and Valve methods work almost identically...
	OOVR_FAILED_OVR_ABORT(ovr_GetInstanceExtensionsVk(*ovr::luid, pchValue, &unBufferSize));
	return unBufferSize;
#else
	STUBBED();
#endif
}

uint32_t BaseCompositor::GetVulkanDeviceExtensionsRequired(VkPhysicalDevice_T * pPhysicalDevice, char * pchValue, uint32_t unBufferSize) {
#if defined(SUPPORT_VK)
	// Use the default LUID, even if another physical device is passed in. TODO.
	OOVR_FAILED_OVR_ABORT(ovr_GetDeviceExtensionsVk(*ovr::luid, pchValue, &unBufferSize));
	return unBufferSize;
#else
	STUBBED();
#endif
}

void BaseCompositor::SetExplicitTimingMode(ovr_enum_t eTimingMode) {
	STUBBED();
}

ovr_enum_t BaseCompositor::SubmitExplicitTimingData() {
	STUBBED();
}

bool BaseCompositor::IsMotionSmoothingSupported() {
	STUBBED();
}

bool BaseCompositor::IsMotionSmoothingEnabled() {
	STUBBED();
}

bool BaseCompositor::IsCurrentSceneFocusAppLoading() {
	STUBBED();
}