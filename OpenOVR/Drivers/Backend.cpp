#include "stdafx.h"

#include "Backend.h"

#include "Misc/Config.h"

// virtual destructor
ITrackedDevice::~ITrackedDevice() {}
IBackend::~IBackend() {}

std::unique_ptr<BackendManager> BackendManager::instance;

void BackendManager::Create(IBackend *backend) {
	instance.reset(new BackendManager());
	instance->backend = std::unique_ptr<IBackend>(backend);
}

BackendManager & BackendManager::Instance() {
	return *instance;
}

void BackendManager::Reset() {
	instance.reset();
}

vr::TrackedDevicePose_t BackendManager::InvalidPose() {
	vr::TrackedDevicePose_t pose = { 0 };
	pose.bPoseIsValid = false;
	pose.bDeviceIsConnected = false;

	return pose;
}

float BackendManager::GetTimeInSeconds() {
#ifdef OC_XR_PORT
	XR_STUBBED();
#else
	return ovr_GetTimeInSeconds();
#endif
}

BackendManager::BackendManager() {
}

BackendManager::~BackendManager() {
}


ITrackedDevice* BackendManager::GetDevice(vr::TrackedDeviceIndex_t index) {
	return backend->GetDevice(index);
}

IHMD* BackendManager::GetPrimaryHMD() {
	return backend->GetPrimaryHMD();
}

void BackendManager::GetSinglePose(
	vr::ETrackingUniverseOrigin origin,
	vr::TrackedDeviceIndex_t index,
	vr::TrackedDevicePose_t * pose,
	ETrackingStateType trackingState) {

	ITrackedDevice *dev = backend->GetDevice(index);

	if (dev) {
		dev->GetPose(origin, pose, trackingState);
	} else {
		*pose = InvalidPose();
	}
}

void BackendManager::GetDeviceToAbsoluteTrackingPose(
	vr::ETrackingUniverseOrigin toOrigin,
	float predictedSecondsToPhotonsFromNow,
	vr::TrackedDevicePose_t * poseArray,
	uint32_t poseArrayCount) {

	backend->GetDeviceToAbsoluteTrackingPose(toOrigin, predictedSecondsToPhotonsFromNow, poseArray, poseArrayCount);
}

// Submitting Frames
void BackendManager::WaitForTrackingData() {
	return backend->WaitForTrackingData();
}

void BackendManager::StoreEyeTexture(
	vr::EVREye eye,
	const vr::Texture_t * texture,
	const vr::VRTextureBounds_t * bounds,
	vr::EVRSubmitFlags submitFlags,
	bool isFirstEye) {

	return backend->StoreEyeTexture(eye, texture, bounds, submitFlags, isFirstEye);
}

void BackendManager::SubmitFrames(bool showSkybox) {
	return backend->SubmitFrames(showSkybox);
}

IBackend::openvr_enum_t BackendManager::SetSkyboxOverride(const vr::Texture_t * pTextures, uint32_t unTextureCount) {
	return backend->SetSkyboxOverride(pTextures, unTextureCount);
}

void BackendManager::ClearSkyboxOverride() {
	return backend->ClearSkyboxOverride();
}

bool BackendManager::GetFrameTiming(OOVR_Compositor_FrameTiming * pTiming, uint32_t unFramesAgo) {
	return backend->GetFrameTiming(pTiming, unFramesAgo);
}

#if defined(SUPPORT_DX)
IBackend::openvr_enum_t BackendManager::GetMirrorTextureD3D11(vr::EVREye eEye, void * pD3D11DeviceOrResource, void ** ppD3D11ShaderResourceView) {
	return backend->GetMirrorTextureD3D11(eEye, pD3D11DeviceOrResource, ppD3D11ShaderResourceView);
}

void BackendManager::ReleaseMirrorTextureD3D11(void * pD3D11ShaderResourceView) {
	return backend->ReleaseMirrorTextureD3D11(pD3D11ShaderResourceView);
}
#endif

bool BackendManager::GetPlayAreaPoints(vr::HmdVector3_t *points, int *count) {
	return backend->GetPlayAreaPoints(points, count);
}

bool BackendManager::AreBoundsVisible() {
	return backend->AreBoundsVisible();
}

void BackendManager::ForceBoundsVisible(bool status){
	return backend->ForceBoundsVisible(status);
}

// ITrackedDevice
bool ITrackedDevice::GetControllerState(vr::VRControllerState_t* state)
{
	// TODO does the length ever change?
	ZeroMemory(state, sizeof(*state));

#ifndef OC_XR_PORT
#error Probably never used, remove
#endif

	// Provide a default implementation for devices that don't supply controller state
	return false;
}

// setup
vr::TrackedDeviceIndex_t ITrackedDevice::DeviceIndex() {
	return deviceIndex;
}

void ITrackedDevice::InitialiseDevice(vr::TrackedDeviceIndex_t index) {
	if (deviceIndex != vr::k_unTrackedDeviceIndexInvalid) {
		OOVR_ABORTF("Cannot initialise tracked device twice - first with ID=%d, then with ID=%d", deviceIndex, index);
	}

	if (index == vr::k_unTrackedDeviceIndexInvalid) {
		OOVR_ABORT("Cannot initialise tracked device with ID k_unTrackedDeviceIndexInvalid");
	}

	deviceIndex = index;
}

int32_t ITrackedDevice::TriggerHapticVibrationAction(float fFrequency, float fAmplitude) {
	// Don't support haptics by default
	// TODO is there a better way to handle this?
	return 0;
}

ITrackedDevice::HandType ITrackedDevice::GetHand()
{
	// By default don't attach to the input system.
	return HAND_NONE;
}

// properties

bool ITrackedDevice::GetBoolTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError * pErrorL) {
	if (oovr_global_configuration.AdmitUnknownProps()) {
		if (pErrorL)
			*pErrorL = vr::TrackedProp_UnknownProperty;
		return false;
	}

	OOVR_ABORTF("unknown bool property - dev: %d, prop: %d", DeviceIndex(), prop);
}

float ITrackedDevice::GetFloatTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError * pErrorL) {
	if (oovr_global_configuration.AdmitUnknownProps()) {
		if (pErrorL)
			*pErrorL = vr::TrackedProp_UnknownProperty;
		return 0;
	}

	OOVR_ABORTF("unknown float property - dev: %d, prop: %d", DeviceIndex(), prop);
}

int32_t ITrackedDevice::GetInt32TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError * pErrorL) {
	if (oovr_global_configuration.AdmitUnknownProps()) {
		*pErrorL = vr::TrackedProp_UnknownProperty;
		return 0;
	}

	OOVR_ABORTF("unknown int32 property - dev: %d, prop: %d", DeviceIndex(), prop);
}

uint64_t ITrackedDevice::GetUint64TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError * pErrorL) {
	if (oovr_global_configuration.AdmitUnknownProps()) {
		*pErrorL = vr::TrackedProp_UnknownProperty;
		return 0;
	}

	OOVR_ABORTF("unknown uint64 property - dev: %d, prop: %d", DeviceIndex(), prop);
}

vr::HmdMatrix34_t ITrackedDevice::GetMatrix34TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError * pErrorL) {
	if (oovr_global_configuration.AdmitUnknownProps()) {
		*pErrorL = vr::TrackedProp_UnknownProperty;

		vr::HmdMatrix34_t m = { 0 };
		m.m[0][0] = 1;
		m.m[1][1] = 1;
		m.m[2][2] = 1;
		return m;
	}

	OOVR_ABORTF("unknown matrix34 property - dev: %d, prop: %d", DeviceIndex(), prop);
}

uint32_t ITrackedDevice::GetArrayTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::PropertyTypeTag_t propType, void * pBuffer, uint32_t unBufferSize, vr::ETrackedPropertyError * pError) {
	if (oovr_global_configuration.AdmitUnknownProps()) {
		*pError = vr::TrackedProp_UnknownProperty;
		return 0;
	}

	OOVR_ABORTF("unknown array property - dev: %d, prop: %d", -1, prop); // TODO use device index
}

uint32_t ITrackedDevice::GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop,
	char * value, uint32_t bufferSize, vr::ETrackedPropertyError * pErrorL) {

	// From docs:
	// input profile to use for this device in the input system. Will default to tracking system
	// name if this isn't provided
	if(prop == vr::Prop_InputProfilePath_String) {
		return GetStringTrackedDeviceProperty(vr::Prop_TrackingSystemName_String, value, bufferSize, pErrorL);
	}

	if (oovr_global_configuration.AdmitUnknownProps()) {
		*pErrorL = vr::TrackedProp_UnknownProperty;
		return 0;
	}

	OOVR_ABORTF("unknown string property - dev: %d, prop: %d", DeviceIndex(), prop);
}
