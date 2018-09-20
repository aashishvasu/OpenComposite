#include "stdafx.h"
#define CVR_IMPL
#include "CVRChaperone003.h"
CVR_GEN_IMPL(CVRChaperone_003);

using namespace std;
using namespace vr;

vr::IVRChaperone_003::ChaperoneCalibrationState CVRChaperone_003::GetCalibrationState() {
	// Casting it directly should work fine for now, and this is unlikely to change
	//  since Chaperone_003 was introduced in 061cf41 (23 Oct. 2015)
	return (vr::IVRChaperone_003::ChaperoneCalibrationState) base.GetCalibrationState();
}
bool CVRChaperone_003::GetPlayAreaSize(float * pSizeX, float * pSizeZ) {
	return base.GetPlayAreaSize(pSizeX, pSizeZ);
}
bool CVRChaperone_003::GetPlayAreaRect(HmdQuad_t * rect) {
	return base.GetPlayAreaRect(rect);
}
void CVRChaperone_003::ReloadInfo() {
	return base.ReloadInfo();
}
void CVRChaperone_003::SetSceneColor(HmdColor_t color) {
	return base.SetSceneColor(color);
}
void CVRChaperone_003::GetBoundsColor(HmdColor_t * pOutputColorArray, int nNumOutputColors, float flCollisionBoundsFadeDistance, HmdColor_t * pOutputCameraColor) {
	return base.GetBoundsColor(pOutputColorArray, nNumOutputColors, flCollisionBoundsFadeDistance, pOutputCameraColor);
}
bool CVRChaperone_003::AreBoundsVisible() {
	return base.AreBoundsVisible();
}
void CVRChaperone_003::ForceBoundsVisible(bool bForce) {
	return base.ForceBoundsVisible(bForce);
}
