#include "stdafx.h"
#define BASE_IMPL
#include "BaseCompositor.h"
#include "BaseInput.h"
#include "BaseOverlay.h"
#include "BaseSystem.h"
#include "Drivers/Backend.h"
#include "Misc/Config.h"
#include "Misc/Haptics.h"
#include "convert.h"
#include "generated/static_bases.gen.h"

#include <string>
#include <cinttypes>
#include <ranges>

#ifdef SUPPORT_DX
#include <dxgi.h> // for GetDefaultAdapterLuid
#pragma comment(lib, "dxgi.lib")
#endif

// Needed for GetOutputDevice if Vulkan is enabled
#if defined(SUPPORT_VK)
#include "../../DrvOpenXR/pub/DrvOpenXR.h"
#ifndef OC_XR_PORT
#include "OVR_CAPI_Vk.h"
#endif
#endif

#include "Misc/xr_ext.h"

namespace {

class PropertyPrinter {
private:
	const vr::ETrackedDeviceProperty prop;
	const vr::TrackedDeviceIndex_t idx;

public:
	PropertyPrinter(vr::ETrackedDeviceProperty prop, vr::TrackedDeviceIndex_t idx, const char* type_name)
	    : prop(prop), idx(idx)
	{
		if (oovr_global_configuration.LogGetTrackedProperty())
			OOVR_LOGF("Requested %s property %u for device %u", type_name, prop, idx);
	}

#define DEF_PRINT_RESULT(type, specifier, expr)                                \
	void print_result(type result)                                         \
	{                                                                      \
		if (oovr_global_configuration.LogGetTrackedProperty())             \
			OOVR_LOGF("dev: %u | prop: %u | result: " specifier, idx, prop, expr); \
	}

	DEF_PRINT_RESULT(bool, "%s", (result) ? "true" : "false")
	DEF_PRINT_RESULT(float, "%f", result)
	DEF_PRINT_RESULT(int32_t, "%" PRIi32, result)
	DEF_PRINT_RESULT(uint64_t, "%" PRIu64, result)
	DEF_PRINT_RESULT(char*, "%s", result)

	void print_result(HmdMatrix34_t result) {
		if (!oovr_global_configuration.LogGetTrackedProperty()) return;
		std::string matrix = "[";
		for (size_t i = 0; i<3; i++){
			matrix.append(" [");
			for (size_t j=0; j<4; j++){
				matrix.append(std::to_string(result.m[i][j]));
				if (j < 3)
					matrix.append(", ");
			}
			matrix.push_back(']');
			if (i<2)
				matrix.push_back(',');
		}
		OOVR_LOGF("dev: %u | prop: %u | result: %s", idx, prop, matrix.c_str());
	}

	template <std::ranges::forward_range T>
	void print_result(const T& result){
		std::string array = "{";
		for (auto item : result){
			array.append(std::to_string(item) + ", ");
		}
		array.erase(array.end() - 2, array.end());
		array.push_back('}');
		OOVR_LOGF("dev: %u | prop: %u | result: %s", idx, prop, array.c_str());
	}

};

} // namespace

BaseSystem::BaseSystem()
{
	// The default origin is now set by the default value of currentSpace
}

void BaseSystem::GetRecommendedRenderTargetSize(uint32_t* width, uint32_t* height)
{
	BackendManager::Instance().GetPrimaryHMD()->GetRecommendedRenderTargetSize(width, height);
}

HmdMatrix44_t BaseSystem::GetProjectionMatrix(EVREye eye, float znear, float zfar)
{
	return BackendManager::Instance().GetPrimaryHMD()->GetProjectionMatrix(eye, znear, zfar);
}

void BaseSystem::GetProjectionRaw(EVREye eye, float* pfLeft, float* pfRight, float* pfTop, float* pfBottom)
{
	return BackendManager::Instance().GetPrimaryHMD()->GetProjectionRaw(eye, pfLeft, pfRight, pfTop, pfBottom);
}

bool BaseSystem::ComputeDistortion(EVREye eEye, float fU, float fV, DistortionCoordinates_t* out)
{
	return BackendManager::Instance().GetPrimaryHMD()->ComputeDistortion(eEye, fU, fV, out);
}

HmdMatrix34_t BaseSystem::GetEyeToHeadTransform(EVREye ovr_eye)
{
	return BackendManager::Instance().GetPrimaryHMD()->GetEyeToHeadTransform(ovr_eye);
}

bool BaseSystem::GetTimeSinceLastVsync(float* pfSecondsSinceLastVsync, uint64_t* pulFrameCounter)
{
	return BackendManager::Instance().GetPrimaryHMD()->GetTimeSinceLastVsync(pfSecondsSinceLastVsync, pulFrameCounter);
}

int32_t BaseSystem::GetD3D9AdapterIndex()
{
	STUBBED();
}

void BaseSystem::GetDXGIOutputInfo(int32_t* adapterIndex)
{
#ifdef SUPPORT_DX

// TODO Turtle1331 use OOVR_ABORT from logging.h
#ifdef WIN32
#define VALIDATE(x, msg)                                                \
	if (!(x)) {                                                         \
		MessageBoxA(nullptr, (msg), "CVRSystem", MB_ICONERROR | MB_OK); \
		exit(-1);                                                       \
	}
#else
#define VALIDATE(x, msg) \
	if (!(x)) {          \
		exit(-42);       \
	}
#endif

	// See the comment on XrExt for why we have to use it
	// FIXME dx12 support?
	XrGraphicsRequirementsD3D11KHR graphicsRequirements = { XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR };
	XrResult res = xr_ext->xrGetD3D11GraphicsRequirementsKHR(xr_instance, xr_system, &graphicsRequirements);
	OOVR_FAILED_XR_ABORT(res);

	IDXGIFactory* DXGIFactory = nullptr;
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&DXGIFactory));
	VALIDATE((hr == ERROR_SUCCESS), "CreateDXGIFactory1 failed");

	bool match = false;
	IDXGIAdapter* Adapter = nullptr;
	for (UINT i = 0; DXGIFactory->EnumAdapters(i, &Adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
		DXGI_ADAPTER_DESC adapterDesc;
		Adapter->GetDesc(&adapterDesc);

		match = memcmp(&adapterDesc.AdapterLuid, &graphicsRequirements.adapterLuid, sizeof(LUID)) == 0;

		Adapter->Release();

		if (match) {
			*adapterIndex = i;
			break;
		}
	}

	// TODO do something with graphicsRequirements.minFeatureLevel

	DXGIFactory->Release();

	if (!match)
		OOVR_ABORT("Cannot find graphics card!");

#undef VALIDATE
#else
	OOVR_ABORT("DX not supported - build with SUPPORT_DX defined");
#endif
}

void BaseSystem::GetOutputDevice(uint64_t* pnDevice, ETextureType textureType, VkInstance_T* pInstance)
{

	switch (textureType) {
	case TextureType_Vulkan: {
#if defined(SUPPORT_VK)
		VkPhysicalDevice physDev;
		DrvOpenXR::VkGetPhysicalDevice(pInstance, &physDev);
		*pnDevice = (uint64_t)physDev;
#else
		OOVR_ABORT("OpenComposite was compiled with Vulkan support disabled, and app attempted to use Vulkan!");
#endif
		break;
	}
	default:
		OOVR_LOGF("Unsupported texture type for GetOutputDevice %d", textureType);
	}
}

bool BaseSystem::IsDisplayOnDesktop()
{
	return false; // Always in direct mode
}

bool BaseSystem::SetDisplayVisibility(bool bIsVisibleOnDesktop)
{
	return false; // Always render in direct mode
}

void BaseSystem::GetDeviceToAbsoluteTrackingPose(ETrackingUniverseOrigin toOrigin, float predictedSecondsToPhotonsFromNow,
    TrackedDevicePose_t* poseArray, uint32_t poseArrayCount)
{

	BackendManager::Instance().GetDeviceToAbsoluteTrackingPose(toOrigin, predictedSecondsToPhotonsFromNow, poseArray, poseArrayCount);
}

HmdMatrix34_t BaseSystem::GetSeatedZeroPoseToStandingAbsoluteTrackingPose()
{
	glm::mat4 m;
	XrSpaceLocation location{ XR_TYPE_SPACE_LOCATION, nullptr, 0, {} };

	OOVR_FAILED_XR_SOFT_ABORT(xrLocateSpace(xr_gbl->seatedSpace, xr_gbl->floorSpace, xr_gbl->GetBestTime(), &location));

	if ((location.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) && (location.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT)) {
		m = glm::mat4(X2G_quat(location.pose.orientation));
		m[3] = glm::vec4(X2G_v3f(location.pose.position), 1.0f);
	}

	return G2S_m34(m);
}

HmdMatrix34_t BaseSystem::GetRawZeroPoseToStandingAbsoluteTrackingPose()
{
	// These *are* the same coordinate systems? They were in LibOVR, let's assume they're the same here too
	return G2S_m34(glm::mat4());
}

uint32_t BaseSystem::GetSortedTrackedDeviceIndicesOfClass(ETrackedDeviceClass targetClass,
    vr::TrackedDeviceIndex_t* indexArray, uint32_t indexCount,
    vr::TrackedDeviceIndex_t unRelativeToTrackedDeviceIndex)
{

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

EDeviceActivityLevel BaseSystem::GetTrackedDeviceActivityLevel(vr::TrackedDeviceIndex_t unDeviceId)
{
	// TODO implement
	return k_EDeviceActivityLevel_UserInteraction;
}

void BaseSystem::ApplyTransform(TrackedDevicePose_t* pOutputPose, const TrackedDevicePose_t* pTrackedDevicePose, const HmdMatrix34_t* pTransform)
{
	OOVR_SOFT_ABORT("ApplyTransform is not implemented! This will probably break things if they're using it!");
}

vr::TrackedDeviceIndex_t BaseSystem::GetTrackedDeviceIndexForControllerRole(vr::ETrackedControllerRole unDeviceType)
{
	if (unDeviceType == TrackedControllerRole_LeftHand) {
		return leftHandIndex;
	} else if (unDeviceType == TrackedControllerRole_RightHand) {
		return rightHandIndex;
	}

	// This is what SteamVR does for unknown devices
	return -1;
}

vr::ETrackedControllerRole BaseSystem::GetControllerRoleForTrackedDeviceIndex(vr::TrackedDeviceIndex_t unDeviceIndex)
{
	if (unDeviceIndex == leftHandIndex) {
		return TrackedControllerRole_LeftHand;
	} else if (unDeviceIndex == rightHandIndex) {
		return TrackedControllerRole_RightHand;
	} else {
		return TrackedControllerRole_Invalid;
	}
}

ETrackedDeviceClass BaseSystem::GetTrackedDeviceClass(vr::TrackedDeviceIndex_t deviceIndex)
{
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

bool BaseSystem::IsTrackedDeviceConnected(vr::TrackedDeviceIndex_t deviceIndex)
{
	// HACK: Beat Saber does not appear to respect controller properties changing without the controller being replugged.
	// The game requests controller properties pretty much immediately after starting, yet we don't know what controllers we have until
	// we pass binding suggestions to the runtime and retrieve the active interaction profile.
	// What we'll do is simulate controllers being unplugged and replugged after actions are loaded, so that controller properties are detected correctly.
	// Note that this means this function probably shouldn't be called from our code for controllers, to avoid messing this up.

	static bool last_loaded[] = { false, false };
	auto actions_loaded = GetUnsafeBaseInput() && GetUnsafeBaseInput()->AreActionsLoaded();
	if (deviceIndex == 1 || deviceIndex == 2) {
		bool ret = actions_loaded == last_loaded[deviceIndex - 1];

		if (!ret) {
			last_loaded[deviceIndex - 1] = actions_loaded;
		}
		return ret;
	}
	return BackendManager::Instance().GetDevice(deviceIndex) != nullptr;
}

bool BaseSystem::GetBoolTrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError* pErrorL)
{
	PropertyPrinter p(prop, unDeviceIndex, "bool");
	ITrackedDevice* dev = BackendManager::Instance().GetDevice(unDeviceIndex);

	if (!dev) {
		*pErrorL = TrackedProp_InvalidDevice;
		return false;
	}

	bool ret = dev->GetBoolTrackedDeviceProperty(prop, pErrorL);
	p.print_result(ret);
	return ret;
}

float BaseSystem::GetFloatTrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError* pErrorL)
{
	PropertyPrinter p(prop, unDeviceIndex, "float");
	ITrackedDevice* dev = BackendManager::Instance().GetDevice(unDeviceIndex);

	if (!dev) {
		*pErrorL = TrackedProp_InvalidDevice;
		return 0;
	}

	float ret = dev->GetFloatTrackedDeviceProperty(prop, pErrorL);
	p.print_result(ret);
	return ret;
}

int32_t BaseSystem::GetInt32TrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError* pErrorL)
{
	PropertyPrinter p(prop, unDeviceIndex, "int32_t");
	ITrackedDevice* dev = BackendManager::Instance().GetDevice(unDeviceIndex);

	if (!dev) {
		*pErrorL = TrackedProp_InvalidDevice;
		return 0;
	}

	int32_t ret = dev->GetInt32TrackedDeviceProperty(prop, pErrorL);
	p.print_result(ret);
	return ret;
}

uint64_t BaseSystem::GetUint64TrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError* pErrorL)
{
	PropertyPrinter p(prop, unDeviceIndex, "uint64_t");
	ITrackedDevice* dev = BackendManager::Instance().GetDevice(unDeviceIndex);

	if (!dev) {
		*pErrorL = TrackedProp_InvalidDevice;
		return 0;
	}

	uint64_t ret = dev->GetUint64TrackedDeviceProperty(prop, pErrorL);
	p.print_result(ret);
	return ret;
}

HmdMatrix34_t BaseSystem::GetMatrix34TrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError* pErrorL)
{
	PropertyPrinter p(prop, unDeviceIndex, "HmdMatrix34_t");
	ITrackedDevice* dev = BackendManager::Instance().GetDevice(unDeviceIndex);

	if (!dev) {
		*pErrorL = TrackedProp_InvalidDevice;
		return { 0 };
	}


	HmdMatrix34_t ret = dev->GetMatrix34TrackedDeviceProperty(prop, pErrorL);
	p.print_result(ret);
	return ret;
}

uint32_t BaseSystem::GetArrayTrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, PropertyTypeTag_t propType, void* pBuffer, uint32_t unBufferSize, ETrackedPropertyError* pError)
{
	PropertyPrinter p(prop, unDeviceIndex, "array");
	ITrackedDevice* dev = BackendManager::Instance().GetDevice(unDeviceIndex);

	if (!dev) {
		*pError = TrackedProp_InvalidDevice;
		return 0;
	}


	uint32_t ret = dev->GetArrayTrackedDeviceProperty(prop, propType, pBuffer, unBufferSize, pError);
	if (oovr_global_configuration.LogGetTrackedProperty()) {
#define ARRAY_CASE(tag_type, actual_type)\
		case k_un##tag_type##PropertyTag:\
			{\
				actual_type* buffer = static_cast<actual_type*>(pBuffer);\
				auto r = std::ranges::subrange(buffer, buffer+unBufferSize);\
				p.print_result(r);\
			}\
			break
		switch (propType) {
			ARRAY_CASE(Float, float);
			ARRAY_CASE(Int32, int32_t);
			ARRAY_CASE(Uint64, uint64_t);
			ARRAY_CASE(Bool, bool);
			ARRAY_CASE(Double, double);
			default:
				OOVR_LOGF("WARNING: Was not able to log array with PropertyTypeTag %" PRIu32 "!" , propType);
				break;
		}
	}
	return ret;
}

uint32_t BaseSystem::GetStringTrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop,
    VR_OUT_STRING() char* value, uint32_t bufferSize, ETrackedPropertyError* pErrorL)
{
	PropertyPrinter p(prop, unDeviceIndex, "string");
	ITrackedDevice* dev = BackendManager::Instance().GetDevice(unDeviceIndex);

	if (!dev) {
		if (pErrorL)
			*pErrorL = TrackedProp_InvalidDevice;
		return 0;
	}

	uint32_t ret = dev->GetStringTrackedDeviceProperty(prop, value, bufferSize, pErrorL);
	p.print_result(value);
	return ret;
}

const char* BaseSystem::GetPropErrorNameFromEnum(ETrackedPropertyError error)
{
#define ERR_CASE(err) \
	case err:         \
		return #err

	switch (error) {
		ERR_CASE(TrackedProp_Success);
		ERR_CASE(TrackedProp_WrongDataType);
		ERR_CASE(TrackedProp_WrongDeviceClass);
		ERR_CASE(TrackedProp_BufferTooSmall);
		ERR_CASE(TrackedProp_UnknownProperty);
		ERR_CASE(TrackedProp_InvalidDevice);
		ERR_CASE(TrackedProp_CouldNotContactServer);
		ERR_CASE(TrackedProp_ValueNotProvidedByDevice);
		ERR_CASE(TrackedProp_StringExceedsMaximumLength);
		ERR_CASE(TrackedProp_NotYetAvailable);
		ERR_CASE(TrackedProp_PermissionDenied);
		ERR_CASE(TrackedProp_InvalidOperation);
		ERR_CASE(TrackedProp_CannotWriteToWildcards);
		ERR_CASE(TrackedProp_IPCReadFailure);
		ERR_CASE(TrackedProp_OutOfMemory);
		ERR_CASE(TrackedProp_InvalidContainer);

	default: {
		// Doesn't work across threads, but in practice shouldn't be an issue.
		static char uknBuf[32];
		memset(uknBuf, 0, sizeof(uknBuf));
		snprintf(uknBuf, sizeof(uknBuf) - 1, "Unknown property error (%d)", (int)error);
		return uknBuf;
	}
	}

#undef ERR_CASE
}

bool BaseSystem::IsInputAvailable()
{
	return BackendManager::Instance().IsInputAvailable();
}

bool BaseSystem::IsSteamVRDrawingControllers()
{
	return !IsInputAvailable();
}

bool BaseSystem::ShouldApplicationPause()
{
	return !IsInputAvailable();
}

bool BaseSystem::ShouldApplicationReduceRenderingWork()
{
	return !IsInputAvailable();
}

void BaseSystem::_OnPostFrame()
{
	frameNumber++;

	// Note: OpenXR event handling is now in XrBackend

	// Create the input system, if the game hasn't already done so
	// See the comment in GetControllerState for the rationale here
	if (!inputSystem) {
		inputSystem = GetBaseInput();

		if (!inputSystem) {
			inputSystem = GetCreateBaseInput();

			// We used to load an empty manifest here. Instead, we now wait until games first read the legacy
			// inputs. This way games that wait until several frames in to load their manifest will still work.
		}
	}

	if (inputSystem) {
		inputSystem->InternalUpdate();
	}
}

void BaseSystem::_EnqueueEvent(const VREvent_t& e)
{
	events.push(e);
}

void BaseSystem::_BlockInputsUntilReleased()
{
	blockingInputsUntilRelease[0] = true;
	blockingInputsUntilRelease[1] = true;
}

float BaseSystem::SGetIpd()
{
	IHMD* dev = BackendManager::Instance().GetPrimaryHMD();
	float ipd = dev->GetIPD();
	static float lastIpd = NAN;
	if (ipd != lastIpd) {
		lastIpd = ipd;
		OOVR_LOGF("IPD: %f", ipd);
	}
	return ipd;
}

void BaseSystem::CheckControllerEvents(TrackedDeviceIndex_t hand, VRControllerState_t& last)
{
	VRControllerState_t state;
	GetControllerState(hand, &state, sizeof(state));

	if (state.ulButtonPressed == last.ulButtonPressed && state.ulButtonTouched == last.ulButtonTouched) {
		// Nothing has changed
		// Though still update other properties in the state
		last = state;

		return;
	}

	VREvent_t ev_base{};
	ev_base.trackedDeviceIndex = hand;
	ev_base.eventAgeSeconds = 0; // TODO
	ev_base.data.controller = { 0 };

	TrackedDevicePose_t pose = { 0 };
	BaseCompositor* compositor = GetUnsafeBaseCompositor();
	if (compositor) {
		compositor->GetSinglePoseRendering(compositor->GetTrackingSpace(), hand, &pose);
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

bool BaseSystem::PollNextEvent(VREvent_t* pEvent, uint32_t uncbVREvent)
{
	return PollNextEventWithPose(TrackingUniverseStanding, pEvent, uncbVREvent, NULL);
}

bool BaseSystem::PollNextEventWithPose(ETrackingUniverseOrigin eOrigin, VREvent_t* pEvent, uint32_t uncbVREvent, vr::TrackedDevicePose_t* pTrackedDevicePose)
{
	memset(pEvent, 0, uncbVREvent);

	if (events.empty()) {
		return false;
	}

	event_info_t info = events.front();
	VREvent_t e = info.ev;
	events.pop();

	memcpy(pEvent, &e, std::min((size_t)uncbVREvent, sizeof(e)));

	if (pTrackedDevicePose) {
		*pTrackedDevicePose = info.pose;
	}

	return true;
}

const char* BaseSystem::GetEventTypeNameFromEnum(EVREventType eType)
{
	STUBBED();
}

HiddenAreaMesh_t BaseSystem::GetHiddenAreaMesh(EVREye eEye, EHiddenAreaMeshType type)
{
	return BackendManager::Instance().GetPrimaryHMD()->GetHiddenAreaMesh(eEye, type);
}

bool BaseSystem::GetControllerState(vr::TrackedDeviceIndex_t controllerDeviceIndex, vr::VRControllerState_t* controllerState, uint32_t controllerStateSize)
{
	if (sizeof(VRControllerState_t) != controllerStateSize)
		OOVR_ABORT("Bad controller state size - was the host compiled with an older version of OpenVR?");

	memset(controllerState, 0, controllerStateSize);

	if (!inputSystem) {
		inputSystem = GetBaseInput();
	}

	// Since we can only bind the manifest once, and some games may never call it or may
	//  try reading the controller state first, wait until the first frame has been submitted (returning
	//  blank controller states until then) AND the game tries reading the controller state.
	// This will work for both games that call this function before loading their manifest (do they really
	//  exist? The comments mentioned that but it seems unlikely one ever actually did, and this bit may
	//  be unnecessary.) and for games that load their manifest late (Jet Island does as of 29/05/2022).
	// This won't work for games that need hand positions but don't use either legacy or new input, but
	//  it seems unlikely any shipping game falls into that category.
	if (!inputSystem) {
		return true;
	}
	if (frameNumber > 0)
		inputSystem->LoadEmptyManifestIfRequired();

	return inputSystem->GetLegacyControllerState(controllerDeviceIndex, controllerState);
}

bool BaseSystem::GetControllerStateWithPose(ETrackingUniverseOrigin eOrigin, vr::TrackedDeviceIndex_t unControllerDeviceIndex,
    vr::VRControllerState_t* pControllerState, uint32_t unControllerStateSize, TrackedDevicePose_t* pTrackedDevicePose)
{

	BackendManager::Instance().GetSinglePose(eOrigin, unControllerDeviceIndex, pTrackedDevicePose, ETrackingStateType::TrackingStateType_Now);

	return GetControllerState(unControllerDeviceIndex, pControllerState, unControllerStateSize);
}

void BaseSystem::TriggerHapticPulse(vr::TrackedDeviceIndex_t unControllerDeviceIndex, uint32_t unAxisId, unsigned short usDurationMicroSec)
{
	if (!oovr_global_configuration.Haptics())
		return;

	if (unControllerDeviceIndex == leftHandIndex || unControllerDeviceIndex == rightHandIndex) {

		// Similar to GetControllerState, wait until the input system is ready
		if (!inputSystem) {
			return;
		}

		inputSystem->TriggerLegacyHapticPulse(unControllerDeviceIndex, (uint64_t)usDurationMicroSec * 1000);
	}
	// if index is invalid, nothing to do
	return;
}

const char* BaseSystem::GetButtonIdNameFromEnum(EVRButtonId eButtonId)
{
#define BTN_CASE(err) \
	case err:         \
		return #err

	switch (eButtonId) {
		BTN_CASE(k_EButton_System);
		BTN_CASE(k_EButton_ApplicationMenu);
		BTN_CASE(k_EButton_Grip);
		BTN_CASE(k_EButton_DPad_Left);
		BTN_CASE(k_EButton_DPad_Up);
		BTN_CASE(k_EButton_DPad_Right);
		BTN_CASE(k_EButton_DPad_Down);
		BTN_CASE(k_EButton_A);
		BTN_CASE(k_EButton_ProximitySensor);
		BTN_CASE(k_EButton_Axis0);
		BTN_CASE(k_EButton_Axis1);
		BTN_CASE(k_EButton_Axis2);
		BTN_CASE(k_EButton_Axis3);
		BTN_CASE(k_EButton_Axis4);

	default: {
		// Doesn't work across threads, but in practice shouldn't be an issue.
		static char uknBuf[32];
		memset(uknBuf, 0, sizeof(uknBuf));
		snprintf(uknBuf, sizeof(uknBuf) - 1, "Unknown EVRButtonId (%d)", (int)eButtonId);
		return uknBuf;
	}
	}

#undef BTN_CASE
}

const char* BaseSystem::GetControllerAxisTypeNameFromEnum(EVRControllerAxisType eAxisType)
{
#define AXIS_CASE(err) \
	case err:          \
		return #err

	switch (eAxisType) {
		AXIS_CASE(k_eControllerAxis_None);
		AXIS_CASE(k_eControllerAxis_TrackPad);
		AXIS_CASE(k_eControllerAxis_Joystick);
		AXIS_CASE(k_eControllerAxis_Trigger);
	default: {
		// Doesn't work across threads, but in practice shouldn't be an issue.
		static char uknBuf[48];
		memset(uknBuf, 0, sizeof(uknBuf));
		snprintf(uknBuf, sizeof(uknBuf) - 1, "Unknown EVRControllerAxisType (%d)", (int)eAxisType);
		return uknBuf;
	}
	}

#undef AXIS_CASE
}

bool BaseSystem::CaptureInputFocus()
{
	STUBBED();
}

void BaseSystem::ReleaseInputFocus()
{
	STUBBED();
}

bool BaseSystem::IsInputFocusCapturedByAnotherProcess()
{
	return !IsInputAvailable();
}

uint32_t BaseSystem::DriverDebugRequest(vr::TrackedDeviceIndex_t unDeviceIndex, const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
{
	STUBBED();
}

vr::EVRFirmwareError BaseSystem::PerformFirmwareUpdate(vr::TrackedDeviceIndex_t unDeviceIndex)
{
	STUBBED();
}

void BaseSystem::AcknowledgeQuit_Exiting()
{
	STUBBED();
}

void BaseSystem::AcknowledgeQuit_UserPrompt()
{
	STUBBED();
}

uint32_t BaseSystem::GetAppContainerFilePaths(VR_OUT_STRING() char* pchBuffer, uint32_t unBufferSize)
{
	STUBBED();
}

const char* BaseSystem::GetRuntimeVersion()
{
	return "1.16.8";
}

DistortionCoordinates_t BaseSystem::ComputeDistortion(EVREye eEye, float fU, float fV)
{
	DistortionCoordinates_t out;
	ComputeDistortion(eEye, fU, fV, &out);
	return out;
}

HmdMatrix44_t BaseSystem::GetProjectionMatrix(EVREye eye, float znear, float zfar, EGraphicsAPIConvention convention)
{
	return BackendManager::Instance().GetPrimaryHMD()->GetProjectionMatrix(eye, znear, zfar, convention);
}

void BaseSystem::PerformanceTestEnableCapture(bool bEnable)
{
	STUBBED();
}

void BaseSystem::PerformanceTestReportFidelityLevelChange(int nFidelityLevel)
{
	STUBBED();
}

// Tracking origin stuff
void BaseSystem::ResetSeatedZeroPose()
{
	if (BackendManager::Instance().IsGraphicsConfigured()) {
		XrSpaceVelocity velocity{ XR_TYPE_SPACE_VELOCITY };
		XrSpaceLocation location{ XR_TYPE_SPACE_LOCATION, &velocity, 0, {} };

		OOVR_FAILED_XR_SOFT_ABORT(xrLocateSpace(xr_gbl->viewSpace, xr_gbl->seatedSpace, xr_gbl->GetBestTime(), &location));

		if ((location.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) && (location.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT)) {
			static XrReferenceSpaceCreateInfo spaceInfo{ XR_TYPE_REFERENCE_SPACE_CREATE_INFO, nullptr, XR_REFERENCE_SPACE_TYPE_LOCAL, { { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f } } };
			XrVector3f rotatedPos;
			auto rot = yRotation(location.pose.orientation, spaceInfo.poseInReferenceSpace.orientation);
			rotate_vector_by_quaternion(location.pose.position, rot, rotatedPos);
			spaceInfo.poseInReferenceSpace.position.x += rotatedPos.x;
			spaceInfo.poseInReferenceSpace.position.y += rotatedPos.y;
			spaceInfo.poseInReferenceSpace.position.z += rotatedPos.z;

			spaceInfo.poseInReferenceSpace.orientation.y = rot.y;
			spaceInfo.poseInReferenceSpace.orientation.w = rot.w;

			spaceInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
			auto oldSpace = xr_gbl->seatedSpace;
			OOVR_FAILED_XR_ABORT(xrCreateReferenceSpace(xr_session, &spaceInfo, &xr_gbl->seatedSpace));
			xrDestroySpace(oldSpace);
		}
	}
}
