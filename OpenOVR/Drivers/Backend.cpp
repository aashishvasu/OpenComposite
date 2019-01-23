#include "stdafx.h"
#include "Backend.h"

// virtual destructor
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

BackendManager::BackendManager() {
}

BackendManager::~BackendManager() {
}


void BackendManager::GetSinglePose(
	vr::ETrackingUniverseOrigin origin,
	vr::TrackedDeviceIndex_t index,
	vr::TrackedDevicePose_t * pose,
	ETrackingStateType trackingState) {

	backend->GetSinglePose(origin, index, pose, trackingState);
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
