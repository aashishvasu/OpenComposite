#include "stdafx.h"
#define BASE_IMPL

#include "Misc/Config.h"

#include "convert.h"

#include <glm/gtx/transform.hpp>

using glm::mat4;
using glm::quat;
using glm::vec3;
using glm::vec4;
using namespace std;

#include "BaseCompositor.h"
#include "BaseOverlay.h"

// For the left and right hand constants - TODO move them to their own file
#include "BaseSystem.h"
#include "static_bases.gen.h"

#ifndef OC_XR_PORT

// Need the LibOVR Vulkan headers for the GetVulkan[Device|Instance]ExtensionsRequired methods
#ifdef SUPPORT_VK
#include "OVR_CAPI_Vk.h"
#endif

#ifdef SUPPORT_DX
#include "OVR_CAPI_D3D.h"
#endif

#endif

// FIXME find a nice way to clean this up
#ifdef SUPPORT_VK
#include "../../DrvOpenXR/pub/DrvOpenXR.h"
#endif

#include "Drivers/Backend.h"
#include "Misc/ScopeGuard.h"

using namespace vr;
using namespace IVRCompositor_022;

typedef int ovr_enum_t;

#define SESS (*ovr::session)

BaseCompositor::BaseCompositor()
{
}

BaseCompositor::~BaseCompositor()
{
}

void BaseCompositor::SetTrackingSpace(ETrackingUniverseOrigin eOrigin)
{
	XrReferenceSpaceType origin = XR_REFERENCE_SPACE_TYPE_STAGE;
	if (eOrigin == TrackingUniverseSeated) {
		origin = XR_REFERENCE_SPACE_TYPE_LOCAL;
	}

	GetUnsafeBaseSystem()->currentSpace = origin;
}

ETrackingUniverseOrigin BaseCompositor::GetTrackingSpace()
{
	if (GetUnsafeBaseSystem()->currentSpace == XR_REFERENCE_SPACE_TYPE_LOCAL) {
		return TrackingUniverseSeated;
	} else {
		return TrackingUniverseStanding;
	}
}

ovr_enum_t BaseCompositor::WaitGetPoses(TrackedDevicePose_t* renderPoseArray, uint32_t renderPoseArrayCount,
    TrackedDevicePose_t* gamePoseArray, uint32_t gamePoseArrayCount)
{

	// Assume this method isn't being called between frames, b/c it really shouldn't be.
	leftEyeSubmitted = false;
	rightEyeSubmitted = false;

	BackendManager::Instance().WaitForTrackingData();

	return GetLastPoses(renderPoseArray, renderPoseArrayCount, gamePoseArray, gamePoseArrayCount);
}

void BaseCompositor::GetSinglePoseRendering(ETrackingUniverseOrigin origin, TrackedDeviceIndex_t unDeviceIndex, TrackedDevicePose_t* pOutputPose)
{
	BackendManager::Instance().GetSinglePose(origin, unDeviceIndex, pOutputPose, ETrackingStateType::TrackingStateType_Rendering);
}

mat4 BaseCompositor::GetHandTransform()
{
	float deg_to_rad = math_pi / 180;

	// The angle offset between the Touch and Vive controllers.
	// If this is incorrect, virtual hands will feel off.
	float controller_offset_angle = 39.5;

	// When testing to try and find the correct value above, uncomment
	//  this to lock the controller perfectly flat.
	// ovrPose.ThePose.Orientation = { 0,0,0,1 };

	vec3 rotateAxis = vec3(1, 0, 0);
	quat rotation = glm::rotate(controller_offset_angle * deg_to_rad, rotateAxis); //count++ * 0.01f);

	mat4 transform(rotation);

	// Controller offset
	// Note this is about right, found by playing around in Unity until everything
	//  roughly lines up. If you want to contribute better numbers, please go ahead!
	transform[3] = vec4(0.0f, 0.0353f, -0.0451f, 1.0f);

	return transform;
}

ovr_enum_t BaseCompositor::GetLastPoses(TrackedDevicePose_t* renderPoseArray, uint32_t renderPoseArrayCount,
    TrackedDevicePose_t* gamePoseArray, uint32_t gamePoseArrayCount)
{

	ETrackingUniverseOrigin origin = GetTrackingSpace();

	for (uint32_t i = 0; i < max(gamePoseArrayCount, renderPoseArrayCount); i++) {
		TrackedDevicePose_t* renderPose = NULL;
		TrackedDevicePose_t* gamePose = NULL;

		if (renderPoseArray) {
			renderPose = i < renderPoseArrayCount ? renderPoseArray + i : NULL;
		}

		if (gamePoseArray) {
			gamePose = i < gamePoseArrayCount ? gamePoseArray + i : NULL;
		}

		if (renderPose) {
			GetSinglePoseRendering(origin, i, renderPose);
		}

		if (gamePose) {
			if (renderPose) {
				*gamePose = *renderPose;
			} else {
				GetSinglePoseRendering(origin, i, gamePose);
			}
		}
	}

	return VRCompositorError_None;
}

ovr_enum_t BaseCompositor::GetLastPoseForTrackedDeviceIndex(TrackedDeviceIndex_t unDeviceIndex, TrackedDevicePose_t* pOutputPose,
    TrackedDevicePose_t* pOutputGamePose)
{

	if (unDeviceIndex < 0 || unDeviceIndex >= k_unMaxTrackedDeviceCount) {
		return VRCompositorError_IndexOutOfRange;
	}

	TrackedDevicePose_t pose{};
	GetSinglePoseRendering(GetTrackingSpace(), unDeviceIndex, &pose);

	if (pOutputPose) {
		*pOutputPose = pose;
	}

	if (pOutputGamePose) {
		*pOutputGamePose = pose;
	}

	return VRCompositorError_None;
}

#if !defined(OC_XR_PORT) && defined(SUPPORT_DX) && defined(SUPPORT_DX11)
DX11Compositor* BaseCompositor::dxcomp;
#endif

Compositor* BaseCompositor::CreateCompositorAPI(const vr::Texture_t* texture)
{
	Compositor* comp = nullptr;

	switch (texture->eType) {
#ifdef SUPPORT_GL
	case TextureType_OpenGL: {
		// Double-cast to avoid a CLion warning
		comp = new GLCompositor((GLuint)(intptr_t)texture->handle);
		break;
	}
#endif
#if defined(SUPPORT_DX) && defined(SUPPORT_DX11)
	case TextureType_DirectX: {
		if (!oovr_global_configuration.DX10Mode())
			comp = new DX11Compositor((ID3D11Texture2D*)texture->handle);

#if defined(SUPPORT_DX10) && !defined(OC_XR_PORT)
		else
			comp = new DX10Compositor((ID3D10Texture2D*)texture->handle);

		dxcomp = (DX11Compositor*)comp;
#else
		else
			STUBBED();
#endif

		break;
	}
#endif
#ifdef SUPPORT_VK
	case TextureType_Vulkan: {
		auto* vk = DrvOpenXR::GetTemporaryVk();
		if (vk == nullptr)
			OOVR_ABORT("Not using temporary Vulkan instance");
		comp = new VkCompositor(texture, vk);
		break;
	}
#endif
#if defined(SUPPORT_DX) && defined(SUPPORT_DX12)
	case TextureType_DirectX12: {
		compositor = new DX12Compositor((D3D12TextureData_t*)texture->handle, fovTextureSize, chains);
		break;
	}
#endif
	default:
		string err = "[BaseCompositor::Submit] Unsupported texture type: " + to_string(texture->eType);
		OOVR_ABORT(err.c_str());
	}

	return comp;
}

ovr_enum_t BaseCompositor::Submit(EVREye eye, const Texture_t* texture, const VRTextureBounds_t* bounds, EVRSubmitFlags submitFlags)
{
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
	} else if (textureNull != isNullRender) {
		OOVR_ABORT("Cannot mismatch first and second eye renders");
	}

	if (!textureNull)
		BackendManager::Instance().StoreEyeTexture(eye, texture, bounds, submitFlags, isFirstEye);

	if (leftEyeSubmitted && rightEyeSubmitted) {
		if (!isNullRender)
			BackendManager::Instance().SubmitFrames(isInSkybox);

		leftEyeSubmitted = false;
		rightEyeSubmitted = false;
	}

	return VRCompositorError_None;
}

void BaseCompositor::ClearLastSubmittedFrame()
{
	// At this point we should show the loading screen and show Guardian, and undo this when the
	// next frame comes along. TODO implement since it would improve loading screens, but it's certainly not critical
}

void BaseCompositor::PostPresentHandoff()
{
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

bool BaseCompositor::GetFrameTiming(OOVR_Compositor_FrameTiming* pTiming, uint32_t unFramesAgo)
{
	return BackendManager::Instance().GetFrameTiming(pTiming, unFramesAgo);

	// TODO fill in the m_nNumVSyncsReadyForUse and uint32_t m_nNumVSyncsToFirstView fields, but only
	// when called from the correct version of the interface.
}

uint32_t BaseCompositor::GetFrameTimings(OOVR_Compositor_FrameTiming* pTiming, uint32_t nFrames)
{
	STUBBED();
}

bool BaseCompositor::GetFrameTiming(vr::Compositor_FrameTiming* pTiming, uint32_t unFramesAgo)
{
	return GetFrameTiming((OOVR_Compositor_FrameTiming*)pTiming, unFramesAgo);
}

uint32_t BaseCompositor::GetFrameTimings(vr::Compositor_FrameTiming* pTiming, uint32_t nFrames)
{
	return GetFrameTimings((OOVR_Compositor_FrameTiming*)pTiming, nFrames);
}

float BaseCompositor::GetFrameTimeRemaining()
{
	STUBBED();
}

void BaseCompositor::GetCumulativeStats(OOVR_Compositor_CumulativeStats* pStats, uint32_t nStatsSizeInBytes)
{
	STUBBED();
}

void BaseCompositor::FadeToColor(float fSeconds, float fRed, float fGreen, float fBlue, float fAlpha, bool bBackground)
{
	fadeTime = fSeconds;
	fadeColour.r = fRed;
	fadeColour.g = fGreen;
	fadeColour.b = fBlue;
	fadeColour.a = fAlpha;

	// TODO what does background do?
}

HmdColor_t BaseCompositor::GetCurrentFadeColor(bool bBackground)
{
	return fadeColour;
}

void BaseCompositor::FadeGrid(float fSeconds, bool bFadeIn)
{
	// This is the app telling SteamVR to fade from the rendered scene into the skybox, eg before the
	//  app loads a new level (this is how the default SteamVR Unity plugin works).
	//
	// Let's not bother implementing the fade (that would be a LOT of work), and just skip straight over.
	isInSkybox = bFadeIn;

	// TODO suppress input while in this mode
}

float BaseCompositor::GetCurrentGridAlpha()
{
	STUBBED();
}

ovr_enum_t BaseCompositor::SetSkyboxOverride(const Texture_t* pTextures, uint32_t unTextureCount)
{
	return BackendManager::Instance().SetSkyboxOverride(pTextures, unTextureCount);
}

void BaseCompositor::ClearSkyboxOverride()
{
	BackendManager::Instance().ClearSkyboxOverride();
}

void BaseCompositor::CompositorBringToFront()
{
	// No actions required, Oculus runs via direct mode
}

void BaseCompositor::CompositorGoToBack()
{
	STUBBED();
}

void BaseCompositor::CompositorQuit()
{
	STUBBED();
}

bool BaseCompositor::IsFullscreen()
{
	STUBBED();
}

uint32_t BaseCompositor::GetCurrentSceneFocusProcess()
{
	STUBBED();
}

uint32_t BaseCompositor::GetLastFrameRenderer()
{
	STUBBED();
}

bool BaseCompositor::CanRenderScene()
{
	return true; // TODO implement
}

void BaseCompositor::ShowMirrorWindow()
{
	STUBBED();
}

void BaseCompositor::HideMirrorWindow()
{
	STUBBED();
}

bool BaseCompositor::IsMirrorWindowVisible()
{
	STUBBED();
}

void BaseCompositor::CompositorDumpImages()
{
	STUBBED();
}

bool BaseCompositor::ShouldAppRenderWithLowResources()
{
	// TODO put in config file
	return false;
}

void BaseCompositor::ForceInterleavedReprojectionOn(bool bOverride)
{
	// Force timewarp on? Yeah right.
}

void BaseCompositor::ForceReconnectProcess()
{
	// We should always be connected
}

void BaseCompositor::SuspendRendering(bool bSuspend)
{
	// TODO
	// I'm not sure what the purpose of this function is. If you know, please tell me.
	// - ZNix
	//STUBBED();
}

ovr_enum_t BaseCompositor::GetMirrorTextureD3D11(EVREye eEye, void* pD3D11DeviceOrResource, void** ppD3D11ShaderResourceView)
{
#if defined(SUPPORT_DX) && defined(SUPPORT_DX11)
	return BackendManager::Instance().GetMirrorTextureD3D11(eEye, pD3D11DeviceOrResource, ppD3D11ShaderResourceView);
#else
	OOVR_ABORT("Cannot get D3D mirror texture - D3D support disabled");
#endif
}

void BaseCompositor::ReleaseMirrorTextureD3D11(void* pD3D11ShaderResourceView)
{
#if defined(SUPPORT_DX) && defined(SUPPORT_DX11)
	return BackendManager::Instance().ReleaseMirrorTextureD3D11(pD3D11ShaderResourceView);
#else
	OOVR_ABORT("Cannot get D3D mirror texture - D3D support disabled");
#endif
}

ovr_enum_t BaseCompositor::GetMirrorTextureGL(EVREye eEye, glUInt_t* pglTextureId, glSharedTextureHandle_t* pglSharedTextureHandle)
{
	STUBBED();
}

bool BaseCompositor::ReleaseSharedGLTexture(glUInt_t glTextureId, glSharedTextureHandle_t glSharedTextureHandle)
{
	STUBBED();
}

void BaseCompositor::LockGLSharedTextureForAccess(glSharedTextureHandle_t glSharedTextureHandle)
{
	STUBBED();
}

void BaseCompositor::UnlockGLSharedTextureForAccess(glSharedTextureHandle_t glSharedTextureHandle)
{
	STUBBED();
}

uint32_t BaseCompositor::GetVulkanInstanceExtensionsRequired(char* pchValue, uint32_t unBufferSize)
{
#if defined(SUPPORT_VK)
	// Whaddya know, the OpenXR, Oculus and Valve methods work almost identically...
	uint32_t size;
	OOVR_FAILED_XR_ABORT(xr_ext->xrGetVulkanInstanceExtensionsKHR(xr_instance, xr_system, unBufferSize, &size, pchValue));
	return size;
#else
	OOVR_ABORT("Vulkan support disabled");
#endif
}

uint32_t BaseCompositor::GetVulkanDeviceExtensionsRequired(VkPhysicalDevice_T* pPhysicalDevice, char* pchValue, uint32_t unBufferSize)
{
#if defined(SUPPORT_VK)
	uint32_t size;
	OOVR_FAILED_XR_ABORT(xr_ext->xrGetVulkanDeviceExtensionsKHR(xr_instance, xr_system, unBufferSize, &size, pchValue));
	return size;
#else
	OOVR_ABORT("Vulkan support disabled");
#endif
}

void BaseCompositor::SetExplicitTimingMode(ovr_enum_t eTimingMode)
{
	// Explicit timing means the application calls SubmitExplicitTimingData each
	// frame, and in return we're not allowed to use their Vulkan queue
	// during WaitGetPoses. We don't do any of that anyway, so nothing needs to
	// be done here.
}

ovr_enum_t BaseCompositor::SubmitExplicitTimingData()
{
	// In SteamVR this records a more accurate timestamp for tracking via the GPU's
	// clock, Oculus doesn't support that so noop here is fine.
	return VRCompositorError_None;
}

bool BaseCompositor::IsMotionSmoothingSupported()
{
	STUBBED();
}

bool BaseCompositor::IsMotionSmoothingEnabled()
{
	STUBBED();
}

bool BaseCompositor::IsCurrentSceneFocusAppLoading()
{
	STUBBED();
}

ovr_enum_t BaseCompositor::SetStageOverride_Async(const char* pchRenderModelPath, const HmdMatrix34_t* pTransform,
    const OOVR_Compositor_StageRenderSettings* pRenderSettings, uint32_t nSizeOfRenderSettings)
{
	STUBBED();
}

void BaseCompositor::ClearStageOverride()
{
	STUBBED();
}

bool BaseCompositor::GetCompositorBenchmarkResults(Compositor_BenchmarkResults* pBenchmarkResults, uint32_t nSizeOfBenchmarkResults)
{
	STUBBED();
}

ovr_enum_t BaseCompositor::GetLastPosePredictionIDs(uint32_t* pRenderPosePredictionID, uint32_t* pGamePosePredictionID)
{
	STUBBED();
}

ovr_enum_t BaseCompositor::GetPosesForFrame(uint32_t unPosePredictionID, TrackedDevicePose_t* pPoseArray, uint32_t unPoseArrayCount)
{
	STUBBED();
}
