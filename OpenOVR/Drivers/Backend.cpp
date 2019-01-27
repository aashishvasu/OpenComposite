#include "stdafx.h"
#include "Backend.h"

// virtual destructor
ITrackedDevice::~ITrackedDevice() {}
IBackend::~IBackend() {}

std::unique_ptr<BackendManager> BackendManager::instance;

void BackendManager::Create(IBackend *backend) {
	instance.reset(new BackendManager());
	instance->backend = backend;
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

BackendManager::BackendManager() {
}

BackendManager::~BackendManager() {
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
