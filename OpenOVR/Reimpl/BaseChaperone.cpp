#include "stdafx.h"
#define BASE_IMPL
#include "BaseChaperone.h"
#include "BaseSystem.h"
#include "static_bases.gen.h"

#include "Drivers/Backend.h"

#include <vector>

using namespace std;
using namespace vr;

BaseChaperone::BaseChaperoneCalibrationState BaseChaperone::GetCalibrationState() {
	return ChaperoneCalibrationState_OK;
}
bool BaseChaperone::GetPlayAreaSize(float *pSizeX, float *pSizeZ) {
	vr::HmdVector3_t minPoint, maxPoint;
	bool success = GetMinMaxPoints(minPoint, maxPoint);

	if(!success)
		return false;

	*pSizeX = maxPoint.v[0] - minPoint.v[0];
	*pSizeZ = maxPoint.v[2] - minPoint.v[2];

	// TODO verify return value
	return true;
}
bool BaseChaperone::GetPlayAreaRect(HmdQuad_t *rect) {
	memset(rect, 0, sizeof(vr::HmdQuad_t));

	vr::HmdVector3_t minPoint, maxPoint;
	bool success = GetMinMaxPoints(minPoint, maxPoint);

	if(!success)
		return false;

	rect->vCorners[0].v[0] = maxPoint.v[0];
	rect->vCorners[0].v[2] = maxPoint.v[2];

	rect->vCorners[1].v[0] = maxPoint.v[0];
	rect->vCorners[1].v[2] = minPoint.v[2];

	rect->vCorners[2].v[0] = minPoint.v[0];
	rect->vCorners[2].v[2] = minPoint.v[2];

	rect->vCorners[3].v[0] = minPoint.v[0];
	rect->vCorners[3].v[2] = maxPoint.v[2];

	return true;
}
void BaseChaperone::ReloadInfo(void) {
	STUBBED();
}
void BaseChaperone::SetSceneColor(HmdColor_t color) {
	STUBBED();
}
void BaseChaperone::GetBoundsColor(HmdColor_t *pOutputColorArray, int nNumOutputColors, float flCollisionBoundsFadeDistance, HmdColor_t *pOutputCameraColor) {
	STUBBED();
}
bool BaseChaperone::AreBoundsVisible() {
	return BackendManager::Instance().AreBoundsVisible();
}
void BaseChaperone::ForceBoundsVisible(bool bForce) {
	return BackendManager::Instance().ForceBoundsVisible(bForce);
}

bool BaseChaperone::GetMinMaxPoints(vr::HmdVector3_t &minPoint, vr::HmdVector3_t &maxPoint) {
	int count;
	bool success = BackendManager::Instance().GetPlayAreaPoints(nullptr, &count);

	if(!success)
		return false; // the play area isn't set up, or is unsupported by the backend

	std::vector<vr::HmdVector3_t> points(count);
	success = BackendManager::Instance().GetPlayAreaPoints(points.data(), nullptr);

	if(!success)
		return false; // shouldn't happen - should be caught by first check

	if(points.size() < 2)
		return false; // not enough points to find a min/max

	minPoint = points[0];
	maxPoint = points[0];

	for(const auto &point : points) {
		for(int i=0; i<3; i++) {
			minPoint.v[i] = min(minPoint.v[i], point.v[i]);
			maxPoint.v[i] = max(maxPoint.v[i], point.v[i]);
		}
	}

	return true;
}

void BaseChaperone::ResetZeroPose(vr::ETrackingUniverseOrigin eTrackingUniverseOrigin)
{
	if (eTrackingUniverseOrigin != TrackingUniverseSeated) {
		STUBBED();
	}

	// TODO do we have to do anything about the tracking origin?
	GetUnsafeBaseSystem()->ResetSeatedZeroPose();
}
