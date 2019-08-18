#include "stdafx.h"
#define BASE_IMPL
#include "BaseSystem.h"
#include "OVR_CAPI.h"
#include "libovr_wrapper.h"
#include "convert.h"
#include "BaseCompositor.h"
#include "BaseOverlay.h"
#include "Misc/Haptics.h"
#include "Misc/Config.h"
#include "static_bases.gen.h"
#include "Drivers/Backend.h"

#include <string>

#ifdef SUPPORT_DX
#include <dxgi.h> // for GetDefaultAdapterLuid
#pragma comment(lib, "dxgi.lib")
#endif

// Needed for GetOutputDevice if Vulkan is enabled
#if defined(SUPPORT_VK)
#include "OVR_CAPI_Vk.h"
#endif

using namespace std;

BaseSystem::BaseSystem() {
	// Set the default origin
	_SetTrackingOrigin(origin);
}

void BaseSystem::GetRecommendedRenderTargetSize(uint32_t * width, uint32_t * height) {
	BackendManager::Instance().GetPrimaryHMD()->GetRecommendedRenderTargetSize(width, height);
}

HmdMatrix44_t BaseSystem::GetProjectionMatrix(EVREye eye, float znear, float zfar) {
	return BackendManager::Instance().GetPrimaryHMD()->GetProjectionMatrix(eye, znear, zfar);
}

void BaseSystem::GetProjectionRaw(EVREye eye, float * pfLeft, float * pfRight, float * pfTop, float * pfBottom) {
	return BackendManager::Instance().GetPrimaryHMD()->GetProjectionRaw(eye, pfLeft, pfRight, pfTop, pfBottom);
}

bool BaseSystem::ComputeDistortion(EVREye eEye, float fU, float fV, DistortionCoordinates_t * out) {
	return BackendManager::Instance().GetPrimaryHMD()->ComputeDistortion(eEye, fU, fV, out);
}

HmdMatrix34_t BaseSystem::GetEyeToHeadTransform(EVREye ovr_eye) {
	return BackendManager::Instance().GetPrimaryHMD()->GetEyeToHeadTransform(ovr_eye);
}

bool BaseSystem::GetTimeSinceLastVsync(float * pfSecondsSinceLastVsync, uint64_t * pulFrameCounter) {
	return BackendManager::Instance().GetPrimaryHMD()->GetTimeSinceLastVsync(pfSecondsSinceLastVsync, pulFrameCounter);
}

int32_t BaseSystem::GetD3D9AdapterIndex() {
	STUBBED();
}

void BaseSystem::GetDXGIOutputInfo(int32_t * adapterIndex) {
#ifdef SUPPORT_DX
#define VALIDATE(x, msg) if (!(x)) { MessageBoxA(nullptr, (msg), "CVRSystem", MB_ICONERROR | MB_OK); exit(-1); }

	LUID* luid = reinterpret_cast<LUID*>(ovr::luid);

	//IDXGIFactory * DXGIFactory = nullptr;
	//HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory), (void**)(&DXGIFactory));
	IDXGIFactory* DXGIFactory = nullptr;
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&DXGIFactory));
	VALIDATE((hr == ERROR_SUCCESS), "CreateDXGIFactory1 failed");

	bool match = false;
	IDXGIAdapter * Adapter = nullptr;
	for (UINT i = 0; DXGIFactory->EnumAdapters(i, &Adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
		DXGI_ADAPTER_DESC adapterDesc;
		Adapter->GetDesc(&adapterDesc);

		match = luid == nullptr || memcmp(&adapterDesc.AdapterLuid, luid, sizeof(LUID)) == 0;

		Adapter->Release();

		if (match) {
			*adapterIndex = i;
			ovr::dxDeviceId = i;
			break;
		}
	}

	DXGIFactory->Release();

	if (!match)
		OOVR_ABORT("Cannot find graphics card!");

#undef VALIDATE
#else
	OOVR_ABORT("DX not supported - build with SUPPORT_DX defined");
#endif
}

void BaseSystem::GetOutputDevice(uint64_t * pnDevice, ETextureType textureType, VkInstance_T * pInstance) {

	switch (textureType) {
	case TextureType_Vulkan: {
#if defined(SUPPORT_VK)
		ovrResult res = ovr_GetSessionPhysicalDeviceVk(*ovr::session, *ovr::luid, pInstance, (VkPhysicalDevice*)pnDevice);
		if (res) {
			OOVR_ABORT("Cannot get Vulkan session physical device!");
		}
#else
		OOVR_ABORT("OpenComposite was compiled with Vulkan support disabled, and app attempted to use Vulkan!");
#endif
	}
	default:
		OOVR_LOGF("Unsupported texture type for GetOutputDevice %d", textureType);
	}

}

bool BaseSystem::IsDisplayOnDesktop() {
	return false; // Always in direct mode
}

bool BaseSystem::SetDisplayVisibility(bool bIsVisibleOnDesktop) {
	return false; // Always render in direct mode
}

void BaseSystem::GetDeviceToAbsoluteTrackingPose(ETrackingUniverseOrigin toOrigin, float predictedSecondsToPhotonsFromNow,
	TrackedDevicePose_t * poseArray, uint32_t poseArrayCount) {

	BackendManager::Instance().GetDeviceToAbsoluteTrackingPose(toOrigin, predictedSecondsToPhotonsFromNow, poseArray, poseArrayCount);
}

HmdMatrix34_t BaseSystem::GetSeatedZeroPoseToStandingAbsoluteTrackingPose() {
	// TODO can we discover the player's seated height somehow?
	// For now just add 0.5 meters
	OVR::Matrix4f in;
	in.Translation(OVR::Vector3f(0, 0.5, 0));

	HmdMatrix34_t res;
	O2S_om34(in, res);
	return res;
}

HmdMatrix34_t BaseSystem::GetRawZeroPoseToStandingAbsoluteTrackingPose() {
	// These *are* the same coordinate systems
	HmdMatrix34_t res;
	O2S_om34(OVR::Matrix4f::Identity(), res);
	return res;
}

uint32_t BaseSystem::GetSortedTrackedDeviceIndicesOfClass(ETrackedDeviceClass targetClass,
		vr::TrackedDeviceIndex_t *indexArray, uint32_t indexCount,
		vr::TrackedDeviceIndex_t unRelativeToTrackedDeviceIndex) {

	uint32_t outCount = 0;

	for (vr::TrackedDeviceIndex_t dev = 0; dev < vr::k_unMaxTrackedDeviceCount; dev++) {
		vr::ETrackedDeviceClass cls = GetTrackedDeviceClass(dev);

		if (cls != targetClass)
			continue;

		if (outCount < indexCount)
			indexArray[outCount] = dev;

		outCount++;
	}

	// TODO sort the devices
	// I'm allowing this to do in despite not sorting it, since I can't see any usecase
	// for relying on the sort results, possibly except for determining the left and right controller.
	//
	// So if some game has the left and right controllers flipped then investigate it, otherwise this
	// shouldn't be a big issue.
	//
	// From what I've seen, it's generally used to get a list of devices of a class, without caring about
	// the order of the results.

	return outCount;
}

EDeviceActivityLevel BaseSystem::GetTrackedDeviceActivityLevel(vr::TrackedDeviceIndex_t unDeviceId) {
	// TODO implement
	return k_EDeviceActivityLevel_UserInteraction;
}

void BaseSystem::ApplyTransform(TrackedDevicePose_t * pOutputPose, const TrackedDevicePose_t * pTrackedDevicePose, const HmdMatrix34_t * pTransform) {
	STUBBED();
}

vr::TrackedDeviceIndex_t BaseSystem::GetTrackedDeviceIndexForControllerRole(vr::ETrackedControllerRole unDeviceType) {
	if (unDeviceType == TrackedControllerRole_LeftHand) {
		return leftHandIndex;
	}
	else if (unDeviceType == TrackedControllerRole_RightHand) {
		return rightHandIndex;
	}
	STUBBED();
}

vr::ETrackedControllerRole BaseSystem::GetControllerRoleForTrackedDeviceIndex(vr::TrackedDeviceIndex_t unDeviceIndex) {
	if (unDeviceIndex == leftHandIndex) {
		return TrackedControllerRole_LeftHand;
	}
	else if (unDeviceIndex == rightHandIndex) {
		return TrackedControllerRole_RightHand;
	}
	else {
		return TrackedControllerRole_Invalid;
	}
}

ETrackedDeviceClass BaseSystem::GetTrackedDeviceClass(vr::TrackedDeviceIndex_t deviceIndex) {
	if (!IsTrackedDeviceConnected(deviceIndex))
		return TrackedDeviceClass_Invalid;

	if (deviceIndex == k_unTrackedDeviceIndex_Hmd)
		return TrackedDeviceClass_HMD;

	if (deviceIndex == leftHandIndex || deviceIndex == rightHandIndex)
		return TrackedDeviceClass_Controller;

	if (deviceIndex == thirdTouchIndex)
		return TrackedDeviceClass_GenericTracker;

	return TrackedDeviceClass_Invalid;
}

bool BaseSystem::IsTrackedDeviceConnected(vr::TrackedDeviceIndex_t deviceIndex) {
	return BackendManager::Instance().GetDevice(deviceIndex) != nullptr;
}

bool BaseSystem::GetBoolTrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError * pErrorL) {
	ITrackedDevice *dev = BackendManager::Instance().GetDevice(unDeviceIndex);

	if (!dev) {
		*pErrorL = TrackedProp_InvalidDevice;
		return false;
	}

	return dev->GetBoolTrackedDeviceProperty(prop, pErrorL);
}

float BaseSystem::GetFloatTrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError * pErrorL) {
	ITrackedDevice *dev = BackendManager::Instance().GetDevice(unDeviceIndex);

	if (!dev) {
		*pErrorL = TrackedProp_InvalidDevice;
		return 0;
	}

	return dev->GetFloatTrackedDeviceProperty(prop, pErrorL);
}

int32_t BaseSystem::GetInt32TrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError * pErrorL) {
	ITrackedDevice *dev = BackendManager::Instance().GetDevice(unDeviceIndex);

	if (!dev) {
		*pErrorL = TrackedProp_InvalidDevice;
		return 0;
	}

	return dev->GetInt32TrackedDeviceProperty(prop, pErrorL);
}

uint64_t BaseSystem::GetUint64TrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError * pErrorL) {
	ITrackedDevice *dev = BackendManager::Instance().GetDevice(unDeviceIndex);

	if (!dev) {
		*pErrorL = TrackedProp_InvalidDevice;
		return 0;
	}

	return dev->GetUint64TrackedDeviceProperty(prop, pErrorL);
}

HmdMatrix34_t BaseSystem::GetMatrix34TrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError * pErrorL) {
	ITrackedDevice *dev = BackendManager::Instance().GetDevice(unDeviceIndex);

	if (!dev) {
		*pErrorL = TrackedProp_InvalidDevice;
		return { 0 };
	}

	return dev->GetMatrix34TrackedDeviceProperty(prop, pErrorL);
}

uint32_t BaseSystem::GetArrayTrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, PropertyTypeTag_t propType, void * pBuffer, uint32_t unBufferSize, ETrackedPropertyError * pError) {
	ITrackedDevice *dev = BackendManager::Instance().GetDevice(unDeviceIndex);

	if (!dev) {
		*pError = TrackedProp_InvalidDevice;
		return { 0 };
	}

	return dev->GetArrayTrackedDeviceProperty(prop, propType, pBuffer, unBufferSize, pError);
}

uint32_t BaseSystem::GetStringTrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop,
	VR_OUT_STRING() char * value, uint32_t bufferSize, ETrackedPropertyError * pErrorL) {

	ITrackedDevice *dev = BackendManager::Instance().GetDevice(unDeviceIndex);

	if (!dev) {
		*pErrorL = TrackedProp_InvalidDevice;
		return 0;
	}

	return dev->GetStringTrackedDeviceProperty(prop, value, bufferSize, pErrorL);
}

const char * BaseSystem::GetPropErrorNameFromEnum(ETrackedPropertyError error) {
	STUBBED();
}

bool BaseSystem::IsInputAvailable() {
	return lastStatus.HasInputFocus;
}

bool BaseSystem::IsSteamVRDrawingControllers() {
	return !lastStatus.HasInputFocus;
}

bool BaseSystem::ShouldApplicationPause() {
	return !lastStatus.HasInputFocus;
}

bool BaseSystem::ShouldApplicationReduceRenderingWork() {
	return lastStatus.OverlayPresent;
}

void BaseSystem::_OnPostFrame() {
	ovrSessionStatus status;
	ovr_GetSessionStatus(*ovr::session, &status);

	if (status.ShouldQuit && !lastStatus.ShouldQuit) {
		VREvent_t e;

		e.eventType = VREvent_Quit;
		e.trackedDeviceIndex = k_unTrackedDeviceIndex_Hmd;
		e.eventAgeSeconds = 0; // Is this required for quit events?

		VREvent_Process_t data;
		data.bForced = false;
		data.pid = data.oldPid = 0; // TODO but probably very rarely used
		e.data.process = data;

		events.push(e);
	}

	CheckControllerEvents(leftHandIndex, lastLeftHandState);
	CheckControllerEvents(rightHandIndex, lastRightHandState);

	// Not exactly an event, but this is a convenient place to put it
	// TODO move all the event handling out and run it per frame, and queue up events
	// Also note this is done after all other events, as it doesn't set ShouldRecenter
	// and thus could end up resetting the pose several times if it occured at the same time
	// as another event
	if (status.ShouldRecenter && !lastStatus.ShouldRecenter) {
		// Why on earth doesn't OpenVR have a recenter event?!
		ResetSeatedZeroPose();
	}

	// Note this isn't called if handle_event is called, preventing one
	//  event from firing despite another event also being changed in the same poll call
	lastStatus = status;
}

void BaseSystem::_EnqueueEvent(const VREvent_t &e) {
	events.push(e);
}

void BaseSystem::_BlockInputsUntilReleased() {
	blockingInputsUntilRelease[0] = true;
	blockingInputsUntilRelease[1] = true;
}

float BaseSystem::SGetIpd() {
	ovrPosef &left = ovr::hmdToEyeViewPose[ovrEye_Left];
	ovrPosef &right = ovr::hmdToEyeViewPose[ovrEye_Right];

	return abs(left.Position.x - right.Position.x);
}

void BaseSystem::CheckControllerEvents(TrackedDeviceIndex_t hand, VRControllerState_t &last) {
	VRControllerState_t state;
	GetControllerState(hand, &state, sizeof(state));

	if (state.ulButtonPressed == last.ulButtonPressed && state.ulButtonTouched == last.ulButtonTouched) {
		// Nothing has changed
		// Though still update other properties in the state
		last = state;

		return;
	}

	VREvent_t ev_base;
	ev_base.trackedDeviceIndex = hand;
	ev_base.eventAgeSeconds = 0; // TODO
	ev_base.data.controller = { 0 };

	TrackedDevicePose_t pose = { 0 };
	BaseCompositor *compositor = GetUnsafeBaseCompositor();
	if (compositor) {
		compositor->GetSinglePoseRendering(origin, hand, &pose);
	}

	// Check each possible button, and fire an event if it changed
	// (note that incrementing enums in C++ is a bit of a pain, wrt the casting)
	for (EVRButtonId id = k_EButton_ApplicationMenu; id < k_EButton_Max; id = (EVRButtonId)(id + 1)) {
		ev_base.data.controller.button = id;
		uint64_t mask = ButtonMaskFromId(id);

		// Was the button pressed or released?
		bool oldState = (last.ulButtonPressed & mask) != 0;
		bool newState = (state.ulButtonPressed & mask) != 0;

		if (newState != oldState) {
			VREvent_t e = ev_base;
			e.eventType = newState ? VREvent_ButtonPress : VREvent_ButtonUnpress;
			events.push(event_info_t(e, pose));
		}

		// Did the user touch or break contact with the button?
		oldState = (last.ulButtonTouched & mask) != 0;
		newState = (state.ulButtonTouched & mask) != 0;

		if (newState != oldState) {
			VREvent_t e = ev_base;
			e.eventType = newState ? VREvent_ButtonTouch : VREvent_ButtonUntouch;
			events.push(event_info_t(e, pose));
		}
	}

	last = state;
}

bool BaseSystem::PollNextEvent(VREvent_t * pEvent, uint32_t uncbVREvent) {
	return PollNextEventWithPose(TrackingUniverseStanding, pEvent, uncbVREvent, NULL);
}

bool BaseSystem::PollNextEventWithPose(ETrackingUniverseOrigin eOrigin, VREvent_t * pEvent, uint32_t uncbVREvent, vr::TrackedDevicePose_t * pTrackedDevicePose) {
	memset(pEvent, 0, uncbVREvent);

	if (events.empty()) {
		return false;
	}

	event_info_t info = events.front();
	VREvent_t e = info.ev;
	events.pop();

	memcpy(pEvent, &e, min(uncbVREvent, sizeof(e)));

	if (pTrackedDevicePose) {
		*pTrackedDevicePose = info.pose;
	}

	return true;
}

const char * BaseSystem::GetEventTypeNameFromEnum(EVREventType eType) {
	STUBBED();
}

HiddenAreaMesh_t BaseSystem::GetHiddenAreaMesh(EVREye eEye, EHiddenAreaMeshType type) {
	return BackendManager::Instance().GetPrimaryHMD()->GetHiddenAreaMesh(eEye, type);
}

bool BaseSystem::GetControllerState(vr::TrackedDeviceIndex_t controllerDeviceIndex, vr::VRControllerState_t * controllerState, uint32_t controllerStateSize) {
	if (sizeof(VRControllerState_t) != controllerStateSize)
		OOVR_ABORT("Bad controller state size - was the host compiled with an older version of OpenVR?");

	memset(controllerState, 0, controllerStateSize);

	ITrackedDevice *dev = BackendManager::Instance().GetDevice(controllerDeviceIndex);

	if(!dev)
		return false;

	bool state = dev->GetControllerState(controllerState);

	if(!state)
		return false;

	// TODO do this properly
	static uint32_t unPacketNum = 0;
	controllerState->unPacketNum = unPacketNum++;

	// Check if we're blocking input
	ovrHandType id = ovrHand_Count;
	if (controllerDeviceIndex == leftHandIndex) {
		id = ovrHand_Left;
	}
	else if (controllerDeviceIndex == rightHandIndex) {
		id = ovrHand_Right;
	}

	if (id != ovrHand_Count && blockingInputsUntilRelease[id]) {
		if (controllerState->ulButtonPressed)
			goto blockInput;

		// Inputs released, permit input again
		blockingInputsUntilRelease[id] = false;
	}

	//  Send the data to the overlays
	BaseOverlay *overlay = GetUnsafeBaseOverlay();
	if (overlay) {
		EVREye side = id == ovrHand_Left ? Eye_Left : Eye_Right;
		bool passToApp = overlay->_HandleOverlayInput(side, controllerDeviceIndex, *controllerState);

		if (!passToApp) {
			goto blockInput;

			// TODO pass this to IsInputFocusCapturedByAnotherProcess
		}
	}

	return true;

blockInput:
	uint32_t packetNum = controllerState->unPacketNum;
	memset(controllerState, 0, controllerStateSize);
	controllerState->unPacketNum = packetNum;
	return true;
}

bool BaseSystem::GetControllerStateWithPose(ETrackingUniverseOrigin eOrigin, vr::TrackedDeviceIndex_t unControllerDeviceIndex,
	vr::VRControllerState_t * pControllerState, uint32_t unControllerStateSize, TrackedDevicePose_t * pTrackedDevicePose) {

	BackendManager::Instance().GetSinglePose(eOrigin, unControllerDeviceIndex, pTrackedDevicePose, ETrackingStateType::TrackingStateType_Now);

	return GetControllerState(unControllerDeviceIndex, pControllerState, unControllerStateSize);
}

void BaseSystem::TriggerHapticPulse(vr::TrackedDeviceIndex_t unControllerDeviceIndex, uint32_t unAxisId, unsigned short usDurationMicroSec) {
	if (!oovr_global_configuration.Haptics())
		return;

	if (unControllerDeviceIndex == leftHandIndex || unControllerDeviceIndex == rightHandIndex) {
		static Haptics haptics;

		haptics.StartSimplePulse(unControllerDeviceIndex == leftHandIndex ? ovrControllerType_LTouch : ovrControllerType_RTouch, usDurationMicroSec);

		return;
	}

	// Invalid controller
	STUBBED();
}

const char * BaseSystem::GetButtonIdNameFromEnum(EVRButtonId eButtonId) {
	STUBBED();
}

const char * BaseSystem::GetControllerAxisTypeNameFromEnum(EVRControllerAxisType eAxisType) {
	STUBBED();
}

bool BaseSystem::CaptureInputFocus() {
	STUBBED();
}

void BaseSystem::ReleaseInputFocus() {
	STUBBED();
}

bool BaseSystem::IsInputFocusCapturedByAnotherProcess() {
	return !lastStatus.HasInputFocus;
}

uint32_t BaseSystem::DriverDebugRequest(vr::TrackedDeviceIndex_t unDeviceIndex, const char * pchRequest, char * pchResponseBuffer, uint32_t unResponseBufferSize) {
	STUBBED();
}

vr::EVRFirmwareError BaseSystem::PerformFirmwareUpdate(vr::TrackedDeviceIndex_t unDeviceIndex) {
	STUBBED();
}

void BaseSystem::AcknowledgeQuit_Exiting() {
	STUBBED();
}

void BaseSystem::AcknowledgeQuit_UserPrompt() {
	STUBBED();
}

uint32_t BaseSystem::GetAppContainerFilePaths(VR_OUT_STRING() char *pchBuffer, uint32_t unBufferSize) {
	STUBBED();
}

const char *BaseSystem::GetRuntimeVersion() {
	STUBBED();
}

DistortionCoordinates_t BaseSystem::ComputeDistortion(EVREye eEye, float fU, float fV) {
	DistortionCoordinates_t out;
	ComputeDistortion(eEye, fU, fV, &out);
	return out;
}

HmdMatrix44_t BaseSystem::GetProjectionMatrix(EVREye eye, float znear, float zfar, EGraphicsAPIConvention convention) {
	return BackendManager::Instance().GetPrimaryHMD()->GetProjectionMatrix(eye, znear, zfar, convention);
}

void BaseSystem::PerformanceTestEnableCapture(bool bEnable) {
	STUBBED();
}

void BaseSystem::PerformanceTestReportFidelityLevelChange(int nFidelityLevel) {
	STUBBED();
}

// Tracking origin stuff
void BaseSystem::_SetTrackingOrigin(ETrackingUniverseOrigin eOrigin) {
	origin = eOrigin;

	ovrTrackingOrigin ovrOrigin = ovrTrackingOrigin_FloorLevel;
	if (eOrigin == TrackingUniverseSeated) {
		ovrOrigin = ovrTrackingOrigin_EyeLevel;
	}

	// When in dual-origin mode, always use the floor level as a base
	if (usingDualOriginMode) {
		ovrOrigin = ovrTrackingOrigin_FloorLevel;
	}

	OOVR_FAILED_OVR_ABORT(ovr_SetTrackingOriginType(*ovr::session, ovrOrigin));
}

ETrackingUniverseOrigin BaseSystem::_GetTrackingOrigin() {
	return origin;
}

HmdMatrix34_t BaseSystem::_PoseToTrackingSpace(ETrackingUniverseOrigin toOrigin, ovrPosef pose) {
	// Standard path, most games only use the origin they have selected
	if (toOrigin == origin && !usingDualOriginMode) {
		goto result;
	}

	if (!usingDualOriginMode) {
		// TODO if usingDualOriginMode is false, then do the initial stuff
		// otherwise the head position will jump around the first time this is used

		// Enable the dual origin mode
		usingDualOriginMode = true;

		// Reset the origin, so LibOVR is now working relative to the floor
		_SetTrackingOrigin(origin);

		// Use the current height as the zero for the seated height
		_ResetFakeSeatedHeight();
	}

	if (toOrigin == TrackingUniverseSeated) {
		pose.Position.y -= fakeOriginHeight;
	}

result:
	OVR::Posef thePose(pose);
	OVR::Matrix4f hmdTransform(thePose);

	HmdMatrix34_t result;
	O2S_om34(hmdTransform, result);
	return result;
}

ETrackingUniverseOrigin BaseSystem::_GetRenderTrackingOrigin() {
	return usingDualOriginMode ? TrackingUniverseStanding : origin;
}

void BaseSystem::_ResetFakeSeatedHeight() {
	ovrTrackingState state = ovr_GetTrackingState(*ovr::session, ovr_GetTimeInSeconds(), false);

	fakeOriginHeight = state.HeadPose.ThePose.Position.y;
}

void BaseSystem::ResetSeatedZeroPose() {
	// TODO should this only work when seated or whatever?
	ovr_RecenterTrackingOrigin(*ovr::session);

	if (usingDualOriginMode)
		_ResetFakeSeatedHeight();
}
