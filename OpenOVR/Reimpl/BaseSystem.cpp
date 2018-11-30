#include "stdafx.h"
#define BASE_IMPL
#include "BaseSystem.h"
#include "OVR_CAPI.h"
#include "libovr_wrapper.h"
#include "convert.h"
#include "BaseCompositor.h"
#include "Misc/Haptics.h"
#include "Misc/Config.h"
#include "static_bases.gen.h"

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
}

void BaseSystem::GetRecommendedRenderTargetSize(uint32_t * width, uint32_t * height) {
	ovrSizei size = ovr_GetFovTextureSize(
		*ovr::session,
		ovrEye_Left, // Resolutions are done per-eye in LibOVR, no particular reason for left eye
		ovr::hmdDesc.DefaultEyeFov[ovrEye_Left],
		oovr_global_configuration.SupersampleRatio()
	);

	*width = size.w;
	*height = size.h;
}

HmdMatrix44_t BaseSystem::GetProjectionMatrix(EVREye eye, float znear, float zfar) {
	return GetProjectionMatrix(eye, znear, zfar, API_DirectX);
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

bool BaseSystem::ComputeDistortion(EVREye eEye, float fU, float fV, DistortionCoordinates_t * out) {
	// Here's what SteamVR does when run on a Rift:
	//  each of the output values match the input values, for all colour channels, regardless of the eye.
	// This is thus essentially a identity transform function.

	if (!out)
		return true;

	out->rfRed[0] = out->rfGreen[0] = out->rfBlue[0] = fU;
	out->rfRed[1] = out->rfGreen[1] = out->rfBlue[1] = fV;

	return true;
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

void BaseSystem::GetDeviceToAbsoluteTrackingPose(ETrackingUniverseOrigin eOrigin, float fPredictedSecondsToPhotonsFromNow,
	TrackedDevicePose_t * pTrackedDevicePoseArray, uint32_t unTrackedDevicePoseArrayCount) {
	STUBBED();
}

void BaseSystem::ResetSeatedZeroPose() {
	// TODO should this only work when seated or whatever?
	ovr_RecenterTrackingOrigin(*ovr::session);
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
	if (deviceIndex == k_unTrackedDeviceIndex_Hmd) {
		return true; // TODO
	}

	unsigned int connected = ovr_GetConnectedControllerTypes(*ovr::session);

	if (oovr_global_configuration.ForceConnectedTouch()) {
		connected |= ovrControllerType_LTouch | ovrControllerType_RTouch;
	}

	if (deviceIndex == leftHandIndex) {
		return connected && ovrControllerType_LTouch != 0;
	}
	else if (deviceIndex == rightHandIndex) {
		return connected && ovrControllerType_RTouch != 0;
	}
	else if (deviceIndex == thirdTouchIndex) {
		return connected && ovrControllerType_Object0 != 0;
	}

	return false;
}

bool BaseSystem::GetBoolTrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError * pErrorL) {
	if (pErrorL)
		*pErrorL = TrackedProp_Success;

	switch (unDeviceIndex) {

		// Motion controllers
	case leftHandIndex:
	case rightHandIndex:
	case thirdTouchIndex:
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

	if (oovr_global_configuration.AdmitUnknownProps()) {
		*pErrorL = TrackedProp_UnknownProperty;
		return 0;
	}

	char msg[1024];
	snprintf(msg, sizeof(msg), "dev: %d, prop: %d", unDeviceIndex, prop);
	OOVR_LOG(msg);

	STUBBED();
}

float BaseSystem::GetFloatTrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError * pErrorL) {
	if (pErrorL)
		*pErrorL = TrackedProp_Success;

	if (unDeviceIndex == k_unTrackedDeviceIndex_Hmd) {
		switch (prop) {
		case Prop_DisplayFrequency_Float:
			return 90.0; // TODO grab this from LibOVR
		case Prop_LensCenterLeftU_Float:
		case Prop_LensCenterLeftV_Float:
		case Prop_LensCenterRightU_Float:
		case Prop_LensCenterRightV_Float:
			// SteamVR reports it as unknown
			*pErrorL = TrackedProp_UnknownProperty;
			return 0;
		case Prop_UserIpdMeters_Float:
			return SGetIpd();
		case Prop_SecondsFromVsyncToPhotons_Float:
			// Seems to be used by croteam games, IDK what the real value is, 100µs should do
			return 0.0001;
		case Prop_UserHeadToEyeDepthMeters_Float:
			// TODO ensure this has the correct sign, though it seems to always be zero anyway
			// In any case, see: https://github.com/ValveSoftware/openvr/issues/398
			return ovr::hmdToEyeViewPose[ovrEye_Left].Position.z;
		}
	}

	if (oovr_global_configuration.AdmitUnknownProps()) {
		*pErrorL = TrackedProp_UnknownProperty;
		return 0;
	}
	
	char msg[1024];
	snprintf(msg, sizeof(msg), "(dev %d): ETrackedDeviceProperty %d", unDeviceIndex, prop);
	OOVR_LOG(msg);
	STUBBED();
}

int32_t BaseSystem::GetInt32TrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError * pErrorL) {
	if (pErrorL)
		*pErrorL = TrackedProp_Success;

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

	if (oovr_global_configuration.AdmitUnknownProps()) {
		*pErrorL = TrackedProp_UnknownProperty;
		return 0;
	}

	char msg[1024];
	snprintf(msg, sizeof(msg), "dev: %d, prop: %d", unDeviceIndex, prop);
	OOVR_LOG(msg);

	STUBBED();
}

uint64_t BaseSystem::GetUint64TrackedDeviceProperty(vr::TrackedDeviceIndex_t dev, ETrackedDeviceProperty prop, ETrackedPropertyError * pErrorL) {
	if(pErrorL)
		*pErrorL = TrackedProp_Success;

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

	if (oovr_global_configuration.AdmitUnknownProps()) {
		*pErrorL = TrackedProp_UnknownProperty;
		return 0;
	}

	char msg[1024];
	snprintf(msg, sizeof(msg), "dev: %d, prop: %d", dev, prop);
	MessageBoxA(NULL, msg, "GetUint64TrackedDeviceProperty", MB_OK);
	STUBBED();
}

HmdMatrix34_t BaseSystem::GetMatrix34TrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError * pErrorL) {
	if (pErrorL)
		*pErrorL = TrackedProp_Success;

	if (oovr_global_configuration.AdmitUnknownProps()) {
		*pErrorL = TrackedProp_UnknownProperty;

		HmdMatrix34_t m = { 0 };
		m.m[0][0] = 1;
		m.m[1][1] = 1;
		m.m[2][2] = 1;
		return m;
	}

	STUBBED();
}

uint32_t BaseSystem::GetArrayTrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, PropertyTypeTag_t propType, void * pBuffer, uint32_t unBufferSize, ETrackedPropertyError * pError) {
	if (pError)
		*pError = TrackedProp_Success;

	if (oovr_global_configuration.AdmitUnknownProps()) {
		*pError = TrackedProp_UnknownProperty;
		return 0;
	}

	STUBBED();
}

uint32_t BaseSystem::GetStringTrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop,
	VR_OUT_STRING() char * value, uint32_t bufferSize, ETrackedPropertyError * pErrorL) {

	if (pErrorL)
		*pErrorL = TrackedProp_Success;

#define PROP(in, out) \
if(prop == in) { \
	if (value != NULL && bufferSize > 0) { \
		strcpy_s(value, bufferSize, out); /* FFS msvc - strncpy IS the secure version of strcpy */ \
	} \
	return (uint32_t) strlen(out) + 1; \
}

	if (oovr_global_configuration.LogGetTrackedProperty()) {
		OOVR_LOGF("(dev %d): ETrackedDeviceProperty %d", unDeviceIndex, prop);
	}

	if (unDeviceIndex == leftHandIndex) {
		PROP(Prop_RenderModelName_String, "renderLeftHand");
	}
	else if(unDeviceIndex == rightHandIndex) {
		PROP(Prop_RenderModelName_String, "renderRightHand");
	}

	// These have been validated against SteamVR
	// TODO add an option to fake this out with 'lighthouse' and 'HTC' in case there is a compatibility issue
	PROP(Prop_TrackingSystemName_String, "oculus");
	PROP(Prop_ManufacturerName_String, "Oculus");

	// Only CV1 has been validated
	switch (ovr::hmdDesc.Type) {
	case ovrHmd_DK1:
		PROP(Prop_ModelNumber_String, "Oculus Rift DK1");
		break;
	case ovrHmd_DK2:
		PROP(Prop_ModelNumber_String, "Oculus Rift DK2");
		break;
	case ovrHmd_CV1:
		PROP(Prop_ModelNumber_String, "Oculus Rift CV1");
		break;
	default:
		PROP(Prop_ModelNumber_String, "<unknown>");
		break;
	}

	// TODO these?
	PROP(Prop_SerialNumber_String, "<unknown>"); // TODO
	PROP(Prop_RenderModelName_String, "<unknown>"); // It appears this just gets passed into IVRRenderModels as the render model name

	// Used by Firebird The Unfinished - see #58
	// Copied from SteamVR
	PROP(Prop_DriverVersion_String, "1.32.0");

#undef PROP

	if (!oovr_global_configuration.AdmitUnknownProps()) {
		OOVR_LOGF("Missing tracked property: dev=%d, ETrackedDeviceProperty=%d", unDeviceIndex, prop);
		OOVR_ABORT("This string property (in log) was not found");
	}

	*pErrorL = TrackedProp_UnknownProperty;
	return 0; // There are tonnes, and we're not implementing all of them.
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
		compositor->GetSinglePoseRendering(hand, &pose);
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

OVR_PUBLIC_FUNCTION(ovrResult)
ovr_GetViewportStencil(
	ovrSession session,
	const ovrViewportStencilDesc* viewportStencilDesc,
	ovrViewportStencilMeshBuffer* outMeshBuffer);

HiddenAreaMesh_t BaseSystem::GetHiddenAreaMesh(EVREye eEye, EHiddenAreaMeshType type) {
	// TODO should we not cache this?
	if (hiddenAreaMeshes[eEye].pVertexData) {
		return hiddenAreaMeshes[eEye];
	}

	if (!oovr_global_configuration.UseViewportStencil()) {
		HiddenAreaMesh_t &result = hiddenAreaMeshes[eEye];
		result.pVertexData = NULL;
		result.unTriangleCount = 0;
		return result;
	}

	ovrEyeType eye = eEye == Eye_Left ? ovrEye_Left : ovrEye_Right;
	ovrViewportStencilDesc desc;
	desc.Eye = eye;
	desc.FovPort = ovr::hmdDesc.DefaultEyeFov[eye];
	desc.HmdToEyeRotation = ovr::eyeRenderDesc[eye].HmdToEyePose.Orientation;

	if (type == k_eHiddenAreaMesh_Inverse) {
		desc.StencilType = ovrViewportStencil_VisibleArea;
	}
	else if (type == k_eHiddenAreaMesh_LineLoop) {
		desc.StencilType = ovrViewportStencil_BorderLine;
	}
	else {
		desc.StencilType = ovrViewportStencil_HiddenArea;
	}

	ovrViewportStencilMeshBuffer mb = { 0 };
	mb.AllocVertexCount = 0;
	mb.VertexBuffer = NULL;
	mb.AllocIndexCount = 0;
	mb.IndexBuffer = NULL;

	// Query for the size
	ovr_GetViewportStencil(*ovr::session, &desc, &mb);

	// Create the buffers
	mb.AllocVertexCount = mb.UsedVertexCount;
	mb.VertexBuffer = new ovrVector2f[mb.AllocVertexCount];
	mb.AllocIndexCount = mb.UsedIndexCount;
	mb.IndexBuffer = new uint16_t[mb.AllocIndexCount];
	
	// Get the data
	ovr_GetViewportStencil(*ovr::session, &desc, &mb);

	// Convert the data into something usable by SteamVR
	HiddenAreaMesh_t &result = hiddenAreaMeshes[eEye];
	vr::HmdVector2_t *arr = new vr::HmdVector2_t[mb.UsedIndexCount];
	result.pVertexData = arr;

	for (int i = 0; i < mb.UsedIndexCount; i++) {
		int index = mb.IndexBuffer[i];
		ovrVector2f &v = mb.VertexBuffer[index];

		arr[i] = HmdVector2_t{v.x, v.y};
	}

	if (type == k_eHiddenAreaMesh_LineLoop) {
		result.unTriangleCount = mb.UsedIndexCount;
	}
	else {
		result.unTriangleCount = mb.UsedIndexCount / 3;
	}

	// Delete the buffers
	delete mb.VertexBuffer;
	delete mb.IndexBuffer;

	// Return the result
	return result;
}

bool BaseSystem::GetControllerState(vr::TrackedDeviceIndex_t controllerDeviceIndex, vr::VRControllerState_t * controllerState, uint32_t controllerStateSize) {
	if (sizeof(VRControllerState_t) != controllerStateSize)
		OOVR_ABORT("Bad controller state size - was the host compiled with an older version of OpenVR?");

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
	var |= ButtonMaskFromId(out)

#define BUTTON(left, right, out) CHECK(Buttons, Button, left, right, out); CHECK(Touches, Touch, left, right, out)

	BUTTON(Y, B, k_EButton_ApplicationMenu);
	BUTTON(X, A, k_EButton_A); // k_EButton_A is the SteamVR name for the lower buttons on the Touch controllers
	BUTTON(LThumb, RThumb, k_EButton_SteamVR_Touchpad);
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

	if (inputState.Touches & (id == ovrHand_Left ? ovrTouch_LIndexTrigger : ovrTouch_RIndexTrigger)) {
		Touches |= ButtonMaskFromId(k_EButton_SteamVR_Trigger);
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
			Buttons |= ButtonMaskFromId(k_EButton_DPad_Right);
		}
		else if (angle < math_pi * 1.0) {
			Buttons |= ButtonMaskFromId(k_EButton_DPad_Down);
		}
		else if (angle < math_pi * 1.5) {
			Buttons |= ButtonMaskFromId(k_EButton_DPad_Left);
		}
		else {
			Buttons |= ButtonMaskFromId(k_EButton_DPad_Up);
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

DistortionCoordinates_t BaseSystem::ComputeDistortion(EVREye eEye, float fU, float fV) {
	DistortionCoordinates_t out;
	ComputeDistortion(eEye, fU, fV, &out);
	return out;
}

HmdMatrix44_t BaseSystem::GetProjectionMatrix(EVREye eye, float znear, float zfar, EGraphicsAPIConvention convention) {
	ovrMatrix4f matrix = ovrMatrix4f_Projection(
		ovr::hmdDesc.DefaultEyeFov[S2O_eye(eye)],
		znear, zfar,
		convention == API_OpenGL ? ovrProjection_ClipRangeOpenGL : ovrProjection_None // TODO is this right?
	);

	return O2S_m4(matrix);
}

void BaseSystem::PerformanceTestEnableCapture(bool bEnable) {
	STUBBED();
}

void BaseSystem::PerformanceTestReportFidelityLevelChange(int nFidelityLevel) {
	STUBBED();
}
