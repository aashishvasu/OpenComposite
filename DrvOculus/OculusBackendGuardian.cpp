#include "DrvOculusCommon.h"
#include "OculusBackend.h"

#include "OVR_CAPI.h"
#include "../OpenOVR/libovr_wrapper.h"

#include <vector>

bool OculusBackend::GetPlayAreaPoints(vr::HmdVector3_t *points, int *count) {
	// Find the number of points
	int pointsCount;
	ovrResult status = ovr_GetBoundaryGeometry(*ovr::session, ovrBoundary_PlayArea, nullptr, &pointsCount);
	OOVR_FAILED_OVR_ABORT(status);

	if(status == ovrSuccess_BoundaryInvalid) {
		if(count)
			*count = 0;
		return false;
	}

	if (count)
		*count = pointsCount;

	if (!points)
		return true;

	std::vector<ovrVector3f> ovrPoints(pointsCount);

	OOVR_FAILED_OVR_ABORT(ovr_GetBoundaryGeometry(
			*ovr::session,
			ovrBoundary_PlayArea,
			ovrPoints.data(),
			&pointsCount
	));

	for (int i = 0; i < pointsCount; i++) {
		points[i].v[0] = ovrPoints[i].x;
		points[i].v[1] = ovrPoints[i].y;
		points[i].v[2] = ovrPoints[i].z;
	}

	return true;
}

bool OculusBackend::AreBoundsVisible() {
	ovrBool out;

	ovrResult res = ovr_GetBoundaryVisible(*ovr::session, &out);
	OOVR_FAILED_OVR_ABORT(res);

	// If the boundaries are invalid, they're surely not visible
	if (res == ovrSuccess_BoundaryInvalid) {
		return false;
	}

	return out;
}

void OculusBackend::ForceBoundsVisible(bool bForce) {
	ovr_RequestBoundaryVisible(*ovr::session, bForce);
}
