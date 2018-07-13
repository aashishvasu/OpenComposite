#include "stdafx.h"
#include "BaseSystem.h"
#include "OVR_CAPI.h"
#include "libovr_wrapper.h"
#include "convert.h"

#include <string>

#define LEFT_HAND_DEVICE_INDEX 1
#define RIGHT_HAND_DEVICE_INDEX 2

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
		ovrProjection_None | ovrProjection_ClipRangeOpenGL // TODO API independent
	);

	return O2S_m4(matrix);
}

void BaseSystem::GetProjectionRaw(EVREye eEye, float * pfLeft, float * pfRight, float * pfTop, float * pfBottom) {
	throw "stub";
}

bool BaseSystem::ComputeDistortion(EVREye eEye, float fU, float fV, DistortionCoordinates_t * pDistortionCoordinates) {
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
	throw "stub";
}

int32_t BaseSystem::GetD3D9AdapterIndex() {
	throw "stub";
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
	throw "stub";
}

bool BaseSystem::IsDisplayOnDesktop() {
	return false; // Always in direct mode
}

bool BaseSystem::SetDisplayVisibility(bool bIsVisibleOnDesktop) {
	return false; // Always render in direct mode
}

void BaseSystem::GetDeviceToAbsoluteTrackingPose(ETrackingUniverseOrigin eOrigin, float fPredictedSecondsToPhotonsFromNow,
	TrackedDevicePose_t * pTrackedDevicePoseArray, uint32_t unTrackedDevicePoseArrayCount) {
	throw "stub";
}

void BaseSystem::ResetSeatedZeroPose() {
	throw "stub";
}

HmdMatrix34_t BaseSystem::GetSeatedZeroPoseToStandingAbsoluteTrackingPose() {
	throw "stub";
}

HmdMatrix34_t BaseSystem::GetRawZeroPoseToStandingAbsoluteTrackingPose() {
	throw "stub";
}

uint32_t BaseSystem::GetSortedTrackedDeviceIndicesOfClass(ETrackedDeviceClass eTrackedDeviceClass,
	vr::TrackedDeviceIndex_t * punTrackedDeviceIndexArray, uint32_t unTrackedDeviceIndexArrayCount,
	vr::TrackedDeviceIndex_t unRelativeToTrackedDeviceIndex) {
	throw "stub";
}

EDeviceActivityLevel BaseSystem::GetTrackedDeviceActivityLevel(vr::TrackedDeviceIndex_t unDeviceId) {
	throw "stub";
}

void BaseSystem::ApplyTransform(TrackedDevicePose_t * pOutputPose, const TrackedDevicePose_t * pTrackedDevicePose, const HmdMatrix34_t * pTransform) {
	throw "stub";
}

vr::TrackedDeviceIndex_t BaseSystem::GetTrackedDeviceIndexForControllerRole(vr::ETrackedControllerRole unDeviceType) {
	throw "stub";
}

vr::ETrackedControllerRole BaseSystem::GetControllerRoleForTrackedDeviceIndex(vr::TrackedDeviceIndex_t unDeviceIndex) {
	throw "stub";
}

ETrackedDeviceClass BaseSystem::GetTrackedDeviceClass(vr::TrackedDeviceIndex_t deviceIndex) {
	if (deviceIndex == k_unTrackedDeviceIndex_Hmd) {
		return TrackedDeviceClass_HMD;
	}

	if (deviceIndex == LEFT_HAND_DEVICE_INDEX || deviceIndex == RIGHT_HAND_DEVICE_INDEX) {
		return TrackedDeviceClass_Controller;
	}

	return TrackedDeviceClass_Invalid;
}

bool BaseSystem::IsTrackedDeviceConnected(vr::TrackedDeviceIndex_t deviceIndex) {
	if (deviceIndex == k_unTrackedDeviceIndex_Hmd) {
		return true; // TODO
	}

	if (deviceIndex == LEFT_HAND_DEVICE_INDEX) {
		unsigned int connected = ovr_GetConnectedControllerTypes(*ovr::session);
		return connected && ovrControllerType_LTouch != 0;
	}
	else if (deviceIndex == RIGHT_HAND_DEVICE_INDEX) {
		unsigned int connected = ovr_GetConnectedControllerTypes(*ovr::session);
		return connected && ovrControllerType_RTouch != 0;
	}

	return false;
}

bool BaseSystem::GetBoolTrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError * pErrorL) {
	throw "stub";
}

float BaseSystem::GetFloatTrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError * pErrorL) {
	throw "stub";
}

int32_t BaseSystem::GetInt32TrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError * pErrorL) {
	throw "stub";
}

uint64_t BaseSystem::GetUint64TrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError * pErrorL) {
	throw "stub";
}

HmdMatrix34_t BaseSystem::GetMatrix34TrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError * pErrorL) {
	throw "stub";
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

	PROP(Prop_TrackingSystemName_String, "Constellation");
	PROP(Prop_SerialNumber_String, "<unknown>"); // TODO

#undef PROP

	return 0; // There are tonnes, and we're not implementing all of them.
}

const char * BaseSystem::GetPropErrorNameFromEnum(ETrackedPropertyError error) {
	throw "stub";
}

bool BaseSystem::PollNextEvent(VREvent_t * pEvent, uint32_t uncbVREvent) {
	return false; // TODO
}

bool BaseSystem::PollNextEventWithPose(ETrackingUniverseOrigin eOrigin, VREvent_t * pEvent, uint32_t uncbVREvent, vr::TrackedDevicePose_t * pTrackedDevicePose) {
	throw "stub";
}

const char * BaseSystem::GetEventTypeNameFromEnum(EVREventType eType) {
	throw "stub";
}

HiddenAreaMesh_t BaseSystem::GetHiddenAreaMesh(EVREye eEye, EHiddenAreaMeshType type) {
	throw "stub";
}

bool BaseSystem::GetControllerState(vr::TrackedDeviceIndex_t controllerDeviceIndex, vr::VRControllerState_t * controllerState, uint32_t controllerStateSize) {
	if (sizeof(VRControllerState_t) != controllerStateSize)
		throw string("Bad controller state size - was the host compiled with an older version of OpenVR?");

	ovrHandType id = ovrHand_Count;

	if (controllerDeviceIndex == LEFT_HAND_DEVICE_INDEX) {
		id = ovrHand_Left;
	}
	else if (controllerDeviceIndex == RIGHT_HAND_DEVICE_INDEX) {
		id = ovrHand_Right;
	}

	if (id == ovrHand_Count) return false;

	uint64_t Buttons = 0;
	uint64_t Touches = 0;

	// TODO cache this
	ovrInputState inputState;
	if (!OVR_SUCCESS(ovr_GetInputState(*ovr::session, ovrControllerType_Touch, &inputState))) return false;

#define CHECK(var, type, left, right, out) \
if(inputState.var && (ovr ## type ## _ ## left || ovr ## type ## _ ## right)) \
	var |= k_EButton_ ## out

#define BUTTON(left, right, out) CHECK(Buttons, Button, left, right, out); CHECK(Touches, Touch, left, right, out)

	BUTTON(A, X, A); // k_EButton_A is the SteamVR name for the lower buttons on the Touch controllers
	BUTTON(B, Y, ApplicationMenu);
	// TODO

#undef BUTTON
#undef CHECK

	controllerState->ulButtonPressed = Buttons;
	controllerState->ulButtonTouched = Touches;
	// TODO

	return true;
}

bool BaseSystem::GetControllerStateWithPose(ETrackingUniverseOrigin eOrigin, vr::TrackedDeviceIndex_t unControllerDeviceIndex,
	vr::VRControllerState_t * pControllerState, uint32_t unControllerStateSize, TrackedDevicePose_t * pTrackedDevicePose) {
	throw "stub";
}

void BaseSystem::TriggerHapticPulse(vr::TrackedDeviceIndex_t unControllerDeviceIndex, uint32_t unAxisId, unsigned short usDurationMicroSec) {
	throw "stub";
}

const char * BaseSystem::GetButtonIdNameFromEnum(EVRButtonId eButtonId) {
	throw "stub";
}

const char * BaseSystem::GetControllerAxisTypeNameFromEnum(EVRControllerAxisType eAxisType) {
	throw "stub";
}

bool BaseSystem::CaptureInputFocus() {
	throw "stub";
}

void BaseSystem::ReleaseInputFocus() {
	throw "stub";
}

bool BaseSystem::IsInputFocusCapturedByAnotherProcess() {
	return false; // TODO
}

uint32_t BaseSystem::DriverDebugRequest(vr::TrackedDeviceIndex_t unDeviceIndex, const char * pchRequest, char * pchResponseBuffer, uint32_t unResponseBufferSize) {
	throw "stub";
}

vr::EVRFirmwareError BaseSystem::PerformFirmwareUpdate(vr::TrackedDeviceIndex_t unDeviceIndex) {
	throw "stub";
}

void BaseSystem::AcknowledgeQuit_Exiting() {
	throw "stub";
}

void BaseSystem::AcknowledgeQuit_UserPrompt() {
	throw "stub";
}
