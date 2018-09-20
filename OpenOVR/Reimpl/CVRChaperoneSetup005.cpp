#include "stdafx.h"
#define CVR_IMPL
#include "CVRChaperoneSetup005.h"
CVR_GEN_IMPL(CVRChaperoneSetup_005);

using namespace vr;

bool CVRChaperoneSetup_005::CommitWorkingCopy(vr::IVRChaperoneSetup_005::EChaperoneConfigFile configFile) {
	// Assume Valve isn't going to change existing enum elements
	return base.CommitWorkingCopy((BaseChaperoneSetup::EChaperoneConfigFile) configFile);
}
void CVRChaperoneSetup_005::RevertWorkingCopy() {
	return base.RevertWorkingCopy();
}
bool CVRChaperoneSetup_005::GetWorkingPlayAreaSize(float * pSizeX, float * pSizeZ) {
	return base.GetWorkingPlayAreaSize(pSizeX, pSizeZ);
}
bool CVRChaperoneSetup_005::GetWorkingPlayAreaRect(HmdQuad_t * rect) {
	return base.GetWorkingPlayAreaRect(rect);
}
bool CVRChaperoneSetup_005::GetWorkingCollisionBoundsInfo(VR_OUT_ARRAY_COUNT(punQuadsCount) HmdQuad_t * pQuadsBuffer, uint32_t* punQuadsCount) {
	return base.GetWorkingCollisionBoundsInfo(pQuadsBuffer, punQuadsCount);
}
bool CVRChaperoneSetup_005::GetLiveCollisionBoundsInfo(VR_OUT_ARRAY_COUNT(punQuadsCount) HmdQuad_t * pQuadsBuffer, uint32_t* punQuadsCount) {
	return base.GetLiveCollisionBoundsInfo(pQuadsBuffer, punQuadsCount);
}
bool CVRChaperoneSetup_005::GetWorkingSeatedZeroPoseToRawTrackingPose(HmdMatrix34_t * pmatSeatedZeroPoseToRawTrackingPose) {
	return base.GetWorkingSeatedZeroPoseToRawTrackingPose(pmatSeatedZeroPoseToRawTrackingPose);
}
bool CVRChaperoneSetup_005::GetWorkingStandingZeroPoseToRawTrackingPose(HmdMatrix34_t * pmatStandingZeroPoseToRawTrackingPose) {
	return base.GetWorkingStandingZeroPoseToRawTrackingPose(pmatStandingZeroPoseToRawTrackingPose);
}
void CVRChaperoneSetup_005::SetWorkingPlayAreaSize(float sizeX, float sizeZ) {
	return base.SetWorkingPlayAreaSize(sizeX, sizeZ);
}
void CVRChaperoneSetup_005::SetWorkingCollisionBoundsInfo(VR_ARRAY_COUNT(unQuadsCount) HmdQuad_t * pQuadsBuffer, uint32_t unQuadsCount) {
	return base.SetWorkingCollisionBoundsInfo(pQuadsBuffer, unQuadsCount);
}
void CVRChaperoneSetup_005::SetWorkingSeatedZeroPoseToRawTrackingPose(const HmdMatrix34_t * pMatSeatedZeroPoseToRawTrackingPose) {
	return base.SetWorkingSeatedZeroPoseToRawTrackingPose(pMatSeatedZeroPoseToRawTrackingPose);
}
void CVRChaperoneSetup_005::SetWorkingStandingZeroPoseToRawTrackingPose(const HmdMatrix34_t * pMatStandingZeroPoseToRawTrackingPose) {
	return base.SetWorkingStandingZeroPoseToRawTrackingPose(pMatStandingZeroPoseToRawTrackingPose);
}
void CVRChaperoneSetup_005::ReloadFromDisk(vr::IVRChaperoneSetup_005::EChaperoneConfigFile configFile) {
	// Assume Valve isn't going to change existing enum elements
	return base.ReloadFromDisk((BaseChaperoneSetup::EChaperoneConfigFile) configFile);
}
bool CVRChaperoneSetup_005::GetLiveSeatedZeroPoseToRawTrackingPose(HmdMatrix34_t * pmatSeatedZeroPoseToRawTrackingPose) {
	return base.GetLiveSeatedZeroPoseToRawTrackingPose(pmatSeatedZeroPoseToRawTrackingPose);
}
void CVRChaperoneSetup_005::SetWorkingCollisionBoundsTagsInfo(VR_ARRAY_COUNT(unTagCount) uint8_t * pTagsBuffer, uint32_t unTagCount) {
	return base.SetWorkingCollisionBoundsTagsInfo(pTagsBuffer, unTagCount);
}
bool CVRChaperoneSetup_005::GetLiveCollisionBoundsTagsInfo(VR_OUT_ARRAY_COUNT(punTagCount) uint8_t * pTagsBuffer, uint32_t * punTagCount) {
	return base.GetLiveCollisionBoundsTagsInfo(pTagsBuffer, punTagCount);
}
bool CVRChaperoneSetup_005::SetWorkingPhysicalBoundsInfo(VR_ARRAY_COUNT(unQuadsCount) HmdQuad_t * pQuadsBuffer, uint32_t unQuadsCount) {
	return base.SetWorkingPhysicalBoundsInfo(pQuadsBuffer, unQuadsCount);
}
bool CVRChaperoneSetup_005::GetLivePhysicalBoundsInfo(VR_OUT_ARRAY_COUNT(punQuadsCount) HmdQuad_t * pQuadsBuffer, uint32_t* punQuadsCount) {
	return base.GetLivePhysicalBoundsInfo(pQuadsBuffer, punQuadsCount);
}
bool CVRChaperoneSetup_005::ExportLiveToBuffer(VR_OUT_STRING() char * pBuffer, uint32_t * pnBufferLength) {
	return base.ExportLiveToBuffer(pBuffer, pnBufferLength);
}
bool CVRChaperoneSetup_005::ImportFromBufferToWorking(const char * pBuffer, uint32_t nImportFlags) {
	return base.ImportFromBufferToWorking(pBuffer, nImportFlags);
}
