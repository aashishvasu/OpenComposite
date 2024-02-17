//
// Created by ZNix on 15/03/2021.
//

#include "stdafx.h"

#include "xrmoreutils.h"
#include <convert.h>

void xr_utils::PoseFromSpace(vr::TrackedDevicePose_t* pose, XrSpace space, vr::ETrackingUniverseOrigin origin,
    std::optional<glm::mat4> extraTransform)
{
	auto baseSpace = xr_space_from_tracking_origin(origin);

	XrSpaceVelocity velocity{ XR_TYPE_SPACE_VELOCITY };
	XrSpaceLocation info{ XR_TYPE_SPACE_LOCATION, &velocity, 0, {} };

	OOVR_FAILED_XR_SOFT_ABORT(xrLocateSpace(space, baseSpace, xr_gbl->GetBestTime(), &info));

	glm::mat4 mat = X2G_om34_pose(info.pose);

	// Apply the extra transform if required - this is applied first, since it's used to swap between the
	// grip and steamvr hand spaces.
	if (extraTransform) {
		mat = mat * extraTransform.value();
	}

	pose->bDeviceIsConnected = true;
	pose->bPoseIsValid = (info.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) != 0;
	pose->mDeviceToAbsoluteTracking = G2S_m34(mat);
	pose->eTrackingResult = pose->bPoseIsValid ? vr::TrackingResult_Running_OK : vr::TrackingResult_Running_OutOfRange;
	pose->vVelocity = X2S_v3f(velocity.linearVelocity); // No offsetting transform - this is in world-space
	pose->vAngularVelocity = X2S_v3f(velocity.angularVelocity); // TODO find out if this needs a transform
}
