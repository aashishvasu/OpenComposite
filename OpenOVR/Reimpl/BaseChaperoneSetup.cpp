#include "stdafx.h"

#define BASE_IMPL
#include "BaseChaperoneSetup.h"

#include "convert.h"

#include <string>

using namespace vr;
using namespace std;

bool BaseChaperoneSetup::CommitWorkingCopy(EChaperoneConfigFile configFile)
{
	STUBBED();
}
void BaseChaperoneSetup::RevertWorkingCopy()
{
	STUBBED();
}
bool BaseChaperoneSetup::GetWorkingPlayAreaSize(float* pSizeX, float* pSizeZ)
{
	STUBBED();
}
bool BaseChaperoneSetup::GetWorkingPlayAreaRect(HmdQuad_t* rect)
{
	STUBBED();
}
bool BaseChaperoneSetup::GetWorkingCollisionBoundsInfo(VR_OUT_ARRAY_COUNT(punQuadsCount) HmdQuad_t* pQuadsBuffer, uint32_t* punQuadsCount)
{
	STUBBED();
}
bool BaseChaperoneSetup::GetLiveCollisionBoundsInfo(VR_OUT_ARRAY_COUNT(punQuadsCount) HmdQuad_t* pQuadsBuffer, uint32_t* punQuadsCount)
{
	using glm::vec3;

	// TODO better find out what this method does

	XrExtent2Df bounds;
	XrResult result = xrGetReferenceSpaceBoundsRect(xr_session, XR_REFERENCE_SPACE_TYPE_STAGE, &bounds);

	if (result == XR_SPACE_BOUNDS_UNAVAILABLE) {
		return false; // TODO verify SteamVR returns this if Guardian isn't set up
	}

	OOVR_FAILED_XR_ABORT(result);

	if (pQuadsBuffer) {
		HmdVector3_t* corners = pQuadsBuffer->vCorners;
		// TODO is this correct? Surely it's offset a bit? What happens when you recentre?
		corners[0] = G2S_v3f(vec3(-bounds.width, 0, -bounds.height));
		corners[1] = G2S_v3f(vec3(-bounds.width, 0, bounds.height));
		corners[2] = G2S_v3f(vec3(bounds.width, 0, bounds.height));
		corners[3] = G2S_v3f(vec3(bounds.width, 0, -bounds.height));
	}

	//string msg = to_string(status) + "," + to_string(pointsCount);
	//OOVR_LOG(msg.c_str());

	if (punQuadsCount)
		*punQuadsCount = 1;

	return true;
}
bool BaseChaperoneSetup::GetWorkingSeatedZeroPoseToRawTrackingPose(HmdMatrix34_t* pmatSeatedZeroPoseToRawTrackingPose)
{
	STUBBED();
}
bool BaseChaperoneSetup::GetWorkingStandingZeroPoseToRawTrackingPose(HmdMatrix34_t* pmatStandingZeroPoseToRawTrackingPose)
{
	STUBBED();
}
void BaseChaperoneSetup::SetWorkingPlayAreaSize(float sizeX, float sizeZ)
{
	// Called by VRGIN (a VR mod framework), to hide Chaperone during seated play. Noop here.
}
void BaseChaperoneSetup::SetWorkingCollisionBoundsInfo(VR_ARRAY_COUNT(unQuadsCount) HmdQuad_t* pQuadsBuffer, uint32_t unQuadsCount)
{
	STUBBED();
}
void BaseChaperoneSetup::SetWorkingSeatedZeroPoseToRawTrackingPose(const HmdMatrix34_t* pMatSeatedZeroPoseToRawTrackingPose)
{
	STUBBED();
}
void BaseChaperoneSetup::SetWorkingStandingZeroPoseToRawTrackingPose(const HmdMatrix34_t* pMatStandingZeroPoseToRawTrackingPose)
{
	STUBBED();
}
void BaseChaperoneSetup::ReloadFromDisk(EChaperoneConfigFile configFile)
{
	STUBBED();
}
bool BaseChaperoneSetup::GetLiveSeatedZeroPoseToRawTrackingPose(HmdMatrix34_t* pmatSeatedZeroPoseToRawTrackingPose)
{
	STUBBED();
}
void BaseChaperoneSetup::SetWorkingCollisionBoundsTagsInfo(VR_ARRAY_COUNT(unTagCount) uint8_t* pTagsBuffer, uint32_t unTagCount)
{
	STUBBED();
}
bool BaseChaperoneSetup::GetLiveCollisionBoundsTagsInfo(VR_OUT_ARRAY_COUNT(punTagCount) uint8_t* pTagsBuffer, uint32_t* punTagCount)
{
	STUBBED();
}
bool BaseChaperoneSetup::SetWorkingPhysicalBoundsInfo(VR_ARRAY_COUNT(unQuadsCount) HmdQuad_t* pQuadsBuffer, uint32_t unQuadsCount)
{
	STUBBED();
}
bool BaseChaperoneSetup::GetLivePhysicalBoundsInfo(VR_OUT_ARRAY_COUNT(punQuadsCount) HmdQuad_t* pQuadsBuffer, uint32_t* punQuadsCount)
{
	STUBBED();
}
bool BaseChaperoneSetup::ExportLiveToBuffer(VR_OUT_STRING() char* pBuffer, uint32_t* pnBufferLength)
{
	STUBBED();
}
bool BaseChaperoneSetup::ImportFromBufferToWorking(const char* pBuffer, uint32_t nImportFlags)
{
	STUBBED();
}
void BaseChaperoneSetup::SetWorkingPerimeter(VR_ARRAY_COUNT(unPointCount) HmdVector2_t* pPointBuffer, uint32_t unPointCount)
{
	STUBBED();
}
void BaseChaperoneSetup::ShowWorkingSetPreview()
{
	STUBBED();
}
void BaseChaperoneSetup::HideWorkingSetPreview()
{
	STUBBED();
}
void BaseChaperoneSetup::RoomSetupStarting()
{
	STUBBED();
}
