#include "stdafx.h"
#define BASE_IMPL
#include "BaseSystem.h"
#include "OVR_CAPI.h"
#include "libovr_wrapper.h"
#include "convert.h"
#include "BaseCompositor.h"
#include "Misc/Haptics.h"

#include <string>

#ifdef SUPPORT_DX
#include <dxgi.h> // for GetDefaultAdapterLuid
#pragma comment(lib, "dxgi.lib")
#endif

using namespace std;

void BaseSystem::GetRecommendedRenderTargetSize(uint32_t * width, uint32_t * height) {
	ovrSizei size = ovr_GetFovTextureSize(
		*ovr::session,
		ovrEye_Left, // Resolutions are done per-eye in LibOVR, no particular reason for left eye
		ovr::hmdDesc.DefaultEyeFov[ovrEye_Left],
		1.0f // 1.0x supersampling default, resulting in no stretched pixels (purpose of this function)
	);

	*width = size.w;
	*height = size.h;
}

HmdMatrix44_t BaseSystem::GetProjectionMatrix(EVREye eye, float znear, float zfar) {
	ovrMatrix4f matrix = ovrMatrix4f_Projection(
		ovr::hmdDesc.DefaultEyeFov[S2O_eye(eye)],
		znear, zfar,
		ovrProjection_None // | ovrProjection_ClipRangeOpenGL // TODO API independent
	);

	return O2S_m4(matrix);
}

void BaseSystem::GetProjectionRaw(EVREye eye, float * pfLeft, float * pfRight, float * pfTop, float * pfBottom) {
	/**
	* With a straight passthrough:
	*
	* SteamVR Left:  -1.110925, 0.889498, -0.964926, 0.715264
	* SteamVR Right: -1.110925, 0.889498, -0.715264, 0.964926
	* OpenOVR Left:  0.889498, 1.110925, 0.964926, 0.715264
	* OpenOVR Right: 0.889498, 1.110925, 0.715264, 0.964926
	*
	* Via:
	*   char buff[1024];
	*   snprintf(buff, sizeof(buff), "eye=%d %f, %f, %f, %f", eye, *pfTop, *pfBottom, *pfLeft, *pfRight);
	*   OOVR_LOG(buff);
	*
	* This suggests that SteamVR negates the top and left values. We should do that too, for obvious reasons.
	*/

	ovrFovPort fov = ovr::hmdDesc.DefaultEyeFov[S2O_eye(eye)];
	*pfTop = -fov.DownTan; // negate, and for some reason the up and down have to be switched
	*pfBottom = fov.UpTan;
	*pfLeft = -fov.LeftTan; // negate
	*pfRight = fov.RightTan;
}

bool BaseSystem::ComputeDistortion(EVREye eEye, float fU, float fV, DistortionCoordinates_t * pDistortionCoordinates) {
	STUBBED();
	return false;
}

HmdMatrix34_t BaseSystem::GetEyeToHeadTransform(EVREye ovr_eye) {
	ovrEyeType eye = S2O_eye(ovr_eye);
	ovrPosef &pose = ovr::hmdToEyeViewPose[eye];

	OVR::Matrix4f transform(pose);
	// For some bizzare reason, inverting the matrix (to go from hmd->eye
	// to eye->hmd) breaks the view, and it's fine without it. That or I'm misunderstanding
	// what exactly this method is supposed to return.

	HmdMatrix34_t result;
	O2S_om34(transform, result);
	return result;
}

bool BaseSystem::GetTimeSinceLastVsync(float * pfSecondsSinceLastVsync, uint64_t * pulFrameCounter) {
	STUBBED();
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
		throw string("Cannot find graphics card!");

#undef VALIDATE
#else
	throw "DX not supported - build with SUPPORT_DX defined";
#endif
}

void BaseSystem::GetOutputDevice(uint64_t * pnDevice, ETextureType textureType, VkInstance_T * pInstance) {
	STUBBED();
}

bool BaseSystem::IsDisplayOnDesktop() {
	return false; // Always in direct mode
}

bool BaseSystem::SetDisplayVisibility(bool bIsVisibleOnDesktop) {
	return false; // Always render in direct mode
}

void BaseSystem::GetDeviceToAbsoluteTrackingPose(ETrackingUniverseOrigin eOrigin, float fPredictedSecondsToPhotonsFromNow,
	TrackedDevicePose_t * pTrackedDevicePoseArray, uint32_t unTrackedDevicePoseArrayCount) {
	STUBBED();
}

void BaseSystem::ResetSeatedZeroPose() {
	// TODO should this only work when seated or whatever?
	ovr_RecenterTrackingOrigin(*ovr::session);
}

HmdMatrix34_t BaseSystem::GetSeatedZeroPoseToStandingAbsoluteTrackingPose() {
	STUBBED();
}

HmdMatrix34_t BaseSystem::GetRawZeroPoseToStandingAbsoluteTrackingPose() {
	STUBBED();
}

uint32_t BaseSystem::GetSortedTrackedDeviceIndicesOfClass(ETrackedDeviceClass eTrackedDeviceClass,
	vr::TrackedDeviceIndex_t * punTrackedDeviceIndexArray, uint32_t unTrackedDeviceIndexArrayCount,
	vr::TrackedDeviceIndex_t unRelativeToTrackedDeviceIndex) {
	STUBBED();
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
	if (deviceIndex == k_unTrackedDeviceIndex_Hmd) {
		return TrackedDeviceClass_HMD;
	}

	if (deviceIndex == leftHandIndex || deviceIndex == rightHandIndex) {
		return TrackedDeviceClass_Controller;
	}

	return TrackedDeviceClass_Invalid;
}

bool BaseSystem::IsTrackedDeviceConnected(vr::TrackedDeviceIndex_t deviceIndex) {
	if (deviceIndex == k_unTrackedDeviceIndex_Hmd) {
		return true; // TODO
	}

	if (deviceIndex == leftHandIndex) {
		unsigned int connected = ovr_GetConnectedControllerTypes(*ovr::session);
		return connected && ovrControllerType_LTouch != 0;
	}
	else if (deviceIndex == rightHandIndex) {
		unsigned int connected = ovr_GetConnectedControllerTypes(*ovr::session);
		return connected && ovrControllerType_RTouch != 0;
	}

	return false;
}

bool BaseSystem::GetBoolTrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError * pErrorL) {
	switch (unDeviceIndex) {

		// Motion controllers
	case leftHandIndex:
	case rightHandIndex:
		switch (prop) {
		case Prop_DeviceProvidesBatteryStatus_Bool:
			return true;
		}
		break;

		// HMD
	case k_unTrackedDeviceIndex_Hmd:
		switch (prop) {
		case Prop_DeviceProvidesBatteryStatus_Bool:
			return false;
		}
		break;
	}

	char msg[1024];
	snprintf(msg, sizeof(msg), "dev: %d, prop: %d", unDeviceIndex, prop);
	OOVR_LOG(msg);

	STUBBED();
}

float BaseSystem::GetFloatTrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError * pErrorL) {
	if (unDeviceIndex == k_unTrackedDeviceIndex_Hmd) {
		switch (prop) {
		case Prop_DisplayFrequency_Float:
			return 90.0; // TODO grab this from LibOVR
		}
	}
	
	char msg[1024];
	snprintf(msg, sizeof(msg), "(dev %d): ETrackedDeviceProperty %d", unDeviceIndex, prop);
	OOVR_LOG(msg);
	STUBBED();
}

int32_t BaseSystem::GetInt32TrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError * pErrorL) {
	// For input mappings, see:
	// https://github.com/jMonkeyEngine/jmonkeyengine/blob/826908b0422d96189ea9827b05ced50d77aadf09/jme3-vr/src/main/java/com/jme3/input/vr/openvr/OpenVRInput.java#L29
	// The rest of the file also contains quite a bit of information about input.

	if (unDeviceIndex == leftHandIndex || unDeviceIndex == rightHandIndex) {
		switch (prop) {
		case Prop_Axis0Type_Int32:
			// TODO find out which of these SteamVR returns and do likewise
			//return k_eControllerAxis_TrackPad;
			return k_eControllerAxis_Joystick;

		case Prop_Axis1Type_Int32:
			return k_eControllerAxis_Trigger;

		case Prop_Axis2Type_Int32:
			return k_eControllerAxis_Trigger;

		case Prop_Axis3Type_Int32:
		case Prop_Axis4Type_Int32:
			return k_eControllerAxis_None;
		}
	}

	char msg[1024];
	snprintf(msg, sizeof(msg), "dev: %d, prop: %d", unDeviceIndex, prop);
	OOVR_LOG(msg);

	STUBBED();
}

uint64_t BaseSystem::GetUint64TrackedDeviceProperty(vr::TrackedDeviceIndex_t dev, ETrackedDeviceProperty prop, ETrackedPropertyError * pErrorL) {
	if (prop == Prop_CurrentUniverseId_Uint64) {
		return 1; // Oculus Rift's universe
	}

	bool is_ctrl = dev == leftHandIndex || dev == rightHandIndex;

	if (is_ctrl && prop == Prop_SupportedButtons_Uint64) {
		return
			ButtonMaskFromId(k_EButton_ApplicationMenu) |
			ButtonMaskFromId(k_EButton_Grip) |
			ButtonMaskFromId(k_EButton_DPad_Left) |
			ButtonMaskFromId(k_EButton_DPad_Up) |
			ButtonMaskFromId(k_EButton_DPad_Down) |
			ButtonMaskFromId(k_EButton_DPad_Right) |
			ButtonMaskFromId(k_EButton_A) |
			ButtonMaskFromId(k_EButton_SteamVR_Touchpad) |
			ButtonMaskFromId(k_EButton_SteamVR_Trigger);
	}

	char msg[1024];
	snprintf(msg, sizeof(msg), "dev: %d, prop: %d", dev, prop);
	MessageBoxA(NULL, msg, "GetUint64TrackedDeviceProperty", MB_OK);
	STUBBED();
}

HmdMatrix34_t BaseSystem::GetMatrix34TrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError * pErrorL) {
	STUBBED();
}

uint32_t BaseSystem::GetArrayTrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, PropertyTypeTag_t propType, void * pBuffer, uint32_t unBufferSize, ETrackedPropertyError * pError) {
	STUBBED();
}

uint32_t BaseSystem::GetStringTrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop,
	VR_OUT_STRING() char * value, uint32_t bufferSize, ETrackedPropertyError * pErrorL) {

#define PROP(in, out) \
if(prop == in) { \
	if (value != NULL && bufferSize > 0) { \
		strcpy_s(value, bufferSize, out); /* FFS msvc - strncpy IS the secure version of strcpy */ \
	} \
	return strlen(out) + 1; \
}

	char str[1024];
	snprintf(str, sizeof(str), "(dev %d): ETrackedDeviceProperty %d", unDeviceIndex, prop);
	OOVR_LOG(str);

	// These have been validated against SteamVR
	// TODO add an option to fake this out with 'lighthouse' and 'HTC' in case there is a compatibility issue
	PROP(Prop_TrackingSystemName_String, "oculus");
	PROP(Prop_ManufacturerName_String, "Oculus");

	// TODO these?
	PROP(Prop_SerialNumber_String, "<unknown>"); // TODO
	PROP(Prop_ModelNumber_String, "<unknown>"); // Serious TODO - some software may not work correctly
	PROP(Prop_RenderModelName_String, "<unknown>"); // It appears this just gets passed into IVRRenderModels as the render model name

#undef PROP

	OOVR_LOG("WARN: This property (previous) was not found");

	STUBBED();

	return 0; // There are tonnes, and we're not implementing all of them.
}

const char * BaseSystem::GetPropErrorNameFromEnum(ETrackedPropertyError error) {
	STUBBED();
}

bool BaseSystem::IsInputAvailable() {
	return true; // TODO
}

bool BaseSystem::IsSteamVRDrawingControllers() {
	return false; // TODO
}

bool BaseSystem::ShouldApplicationPause() {
	return false; // TODO
}

bool BaseSystem::ShouldApplicationReduceRenderingWork() {
	return false; // TODO
}

void BaseSystem::CheckEvents() {
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

bool BaseSystem::PollNextEvent(VREvent_t * pEvent, uint32_t uncbVREvent) {
	CheckEvents();

	memset(pEvent, 0, uncbVREvent);

	if (events.empty()) {
		return false;
	}

	VREvent_t e = events.front();
	events.pop();

	memcpy(pEvent, &e, min(uncbVREvent, sizeof(e)));
	return true;
}

bool BaseSystem::PollNextEventWithPose(ETrackingUniverseOrigin eOrigin, VREvent_t * pEvent, uint32_t uncbVREvent, vr::TrackedDevicePose_t * pTrackedDevicePose) {
	STUBBED();
}

const char * BaseSystem::GetEventTypeNameFromEnum(EVREventType eType) {
	STUBBED();
}

HiddenAreaMesh_t BaseSystem::GetHiddenAreaMesh(EVREye eEye, EHiddenAreaMeshType type) {
	// TODO implement

	HiddenAreaMesh_t result;
	result.pVertexData = NULL;
	result.unTriangleCount = 0;

	return result;
}

bool BaseSystem::GetControllerState(vr::TrackedDeviceIndex_t controllerDeviceIndex, vr::VRControllerState_t * controllerState, uint32_t controllerStateSize) {
	if (sizeof(VRControllerState_t) != controllerStateSize)
		throw string("Bad controller state size - was the host compiled with an older version of OpenVR?");

	ovrHandType id = ovrHand_Count;

	if (controllerDeviceIndex == leftHandIndex) {
		id = ovrHand_Left;
	}
	else if (controllerDeviceIndex == rightHandIndex) {
		id = ovrHand_Right;
	}

	if (id == ovrHand_Count) return false;

	uint64_t Buttons = 0;
	uint64_t Touches = 0;

	// TODO cache this
	ovrInputState inputState;
	ovrResult result = ovr_GetInputState(*ovr::session, ovrControllerType_Touch, &inputState);
	if (!OVR_SUCCESS(result)) {
		string str = "[WARN] Could not get input: ";
		str += to_string(result);
		OOVR_LOG(str.c_str());
		return false;
	}

#define CHECK(var, type, left, right, out) \
if(inputState.var & (id == ovrHand_Left ? ovr ## type ## _ ## left : ovr ## type ## _ ## right)) \
	var |= ButtonMaskFromId(k_EButton_ ## out)

#define BUTTON(left, right, out) CHECK(Buttons, Button, left, right, out); CHECK(Touches, Touch, left, right, out)

	BUTTON(A, X, A); // k_EButton_A is the SteamVR name for the lower buttons on the Touch controllers
	BUTTON(B, Y, ApplicationMenu);
	BUTTON(LThumb, RThumb, SteamVR_Touchpad);
	// TODO

#undef BUTTON
#undef CHECK

	// Grip/Trigger button
	// TODO what should the cutoff be?
	if (inputState.HandTrigger[id] >= 0.4) {
		Buttons |= ButtonMaskFromId(k_EButton_Grip);
	}
	if (inputState.IndexTrigger[id] >= 0.4) {
		Buttons |= ButtonMaskFromId(k_EButton_SteamVR_Trigger);
	}

	// Trigger and Thumbstick - Analog (axis) inputs
	VRControllerAxis_t &trigger = controllerState->rAxis[1];
	trigger.x = inputState.IndexTrigger[id];
	trigger.y = 0;

	VRControllerAxis_t &grip = controllerState->rAxis[2];
	grip.x = inputState.HandTrigger[id];
	grip.y = 0;

	VRControllerAxis_t &thumbstick = controllerState->rAxis[0];
	ovrVector2f &ovrThumbstick = inputState.Thumbstick[id];
	thumbstick.x = ovrThumbstick.x;
	thumbstick.y = ovrThumbstick.y;

	// Pythagoras, and don't bother square rooting it since that's much slower than squaring what we compare it to
	float valueSquared = thumbstick.x * thumbstick.x + thumbstick.y * thumbstick.y;

	// The threshold for activating the virtual DPad buttons
	// TODO add a latch thing so you can't have it flip back and forth
	float threshold = 0.6f;

	if (valueSquared > threshold * threshold) {
		// 0=west
		float angle = atan2(thumbstick.y, thumbstick.x);

		// Subtract 45deg so the divisions are diagonal
		angle -= math_pi / 4;

		if (angle < 0)
			angle += math_pi * 2;

		if (angle < math_pi * 0.5) {
			Buttons = ButtonMaskFromId(k_EButton_DPad_Right);
		}
		else if (angle < math_pi * 1.0) {
			Buttons = ButtonMaskFromId(k_EButton_DPad_Down);
		}
		else if (angle < math_pi * 1.5) {
			Buttons = ButtonMaskFromId(k_EButton_DPad_Left);
		}
		else {
			Buttons = ButtonMaskFromId(k_EButton_DPad_Up);
		}
	}

	controllerState->ulButtonPressed = Buttons;
	controllerState->ulButtonTouched = Touches;

	// TODO do this properly
	static uint32_t unPacketNum = 0;
	controllerState->unPacketNum = unPacketNum++;

	return true;
}

bool BaseSystem::GetControllerStateWithPose(ETrackingUniverseOrigin eOrigin, vr::TrackedDeviceIndex_t unControllerDeviceIndex,
	vr::VRControllerState_t * pControllerState, uint32_t unControllerStateSize, TrackedDevicePose_t * pTrackedDevicePose) {

	ovrTrackingState trackingState = ovr_GetTrackingState(*ovr::session, 0 /* Most recent */, ovrTrue);
	BaseCompositor::GetSinglePose(unControllerDeviceIndex, pTrackedDevicePose, trackingState);

	// TODO handle eOrigin

	return GetControllerState(unControllerDeviceIndex, pControllerState, unControllerStateSize);
}

void BaseSystem::TriggerHapticPulse(vr::TrackedDeviceIndex_t unControllerDeviceIndex, uint32_t unAxisId, unsigned short usDurationMicroSec) {
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
	return false; // TODO
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
