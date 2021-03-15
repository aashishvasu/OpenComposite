//
// Like xrutil.h but not included in every file by default
//
// Created by ZNix on 15/03/2021.
//

#pragma once

#include <OpenVR/interfaces/vrtypes.h>
#include <openxr/openxr.h>

namespace xr_utils {

void PoseFromSpace(vr::TrackedDevicePose_t* pose, XrSpace space, vr::ETrackingUniverseOrigin origin);

}
