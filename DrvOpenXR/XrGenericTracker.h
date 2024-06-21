#pragma once

#include "../RuntimeExtensions/XR_MNDX_xdev_space.h"
#include "Misc/Input/InteractionProfile.h"
#include "XrTrackedDevice.h"

#include <set>
#include <string>

constexpr int MAX_GENERIC_TRACKERS = vr::k_unMaxTrackedDeviceCount - RESERVED_DEVICE_INDICES;

class XrGenericTracker : public virtual XrTrackedDevice {
public:
	explicit XrGenericTracker(const InteractionProfile& profile, XrXDevPropertiesMNDX properties, uint32_t index, XrSpace space);

	void GetPose(
	    vr::ETrackingUniverseOrigin origin,
	    vr::TrackedDevicePose_t* pose,
	    ETrackingStateType trackingState) override;

	vr::ETrackedDeviceClass GetTrackedDeviceClass() override;
	const InteractionProfile* GetInteractionProfile() override;

	TrackedDeviceType GetHand() override;

	uint64_t GetUint64TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pErrorL) override;
	uint32_t GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, char* pchValue, uint32_t unBufferSize, vr::ETrackedPropertyError* pErrorL) override;

private:
	const InteractionProfile& profile;
	XrXDevPropertiesMNDX xdevProperties;
	uint32_t genericTrackerIndex;
	XrSpace genericTrackerSpace;
};
