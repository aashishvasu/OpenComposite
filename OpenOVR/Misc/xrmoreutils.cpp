//
// Created by ZNix on 15/03/2021.
//

#include "stdafx.h"

#include "xrmoreutils.h"
#include <convert.h>

void xr_utils::PoseFromSpace(vr::TrackedDevicePose_t* pose, XrSpace space, vr::ETrackingUniverseOrigin origin)
{
	auto baseSpace = xr_space_from_tracking_origin(origin);

	XrSpaceVelocity velocity{ XR_TYPE_SPACE_VELOCITY };
	XrSpaceLocation info{ XR_TYPE_SPACE_LOCATION, &velocity };

	XrResult xrLocateSpace_res = xrLocateSpace(space, baseSpace, xr_gbl->GetBestTime(), &info);
	if (xrLocateSpace_res != XR_SUCCESS) {
		OOVR_SOFT_ABORT("xrLocateSpace failed: %d", xrLocateSpace_res);
		pose->bPoseIsValid = false;
	}

	// TODO velocity
	pose->bDeviceIsConnected = true;
	pose->bPoseIsValid = (info.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) != 0;
	pose->mDeviceToAbsoluteTracking = G2S_m34(X2G_om34_pose(info.pose));
	pose->eTrackingResult = pose->bPoseIsValid ? vr::TrackingResult_Running_OK : vr::TrackingResult_Running_OutOfRange;
}
