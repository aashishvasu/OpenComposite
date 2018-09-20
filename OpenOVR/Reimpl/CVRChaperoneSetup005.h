#pragma once
#include "BaseChaperoneSetup.h"
#include "OpenVR/interfaces/IVRChaperoneSetup_005.h"

/** Manages the working copy of the chaperone info. By default this will be the same as the
* live copy. Any changes made with this interface will stay in the working copy until
* CommitWorkingCopy() is called, at which point the working copy and the live copy will be
* the same again. */
class CVRChaperoneSetup_005 : public vr::IVRChaperoneSetup_005::IVRChaperoneSetup, public CVRCommon {
	CVR_GEN_IFACE();

private:
	BaseChaperoneSetup base;
public:

	/** Saves the current working copy to disk */
	INTERFACE_FUNC(bool, CommitWorkingCopy, vr::IVRChaperoneSetup_005::EChaperoneConfigFile configFile);

	/** Reverts the working copy to match the live chaperone calibration.
	* To modify existing data this MUST be do WHILE getting a non-error ChaperoneCalibrationStatus.
	* Only after this should you do gets and sets on the existing data. */
	INTERFACE_FUNC(void, RevertWorkingCopy);

	/** Returns the width and depth of the Play Area (formerly named Soft Bounds) in X and Z from the working copy.
	* Tracking space center (0,0,0) is the center of the Play Area. */
	INTERFACE_FUNC(bool, GetWorkingPlayAreaSize, float *pSizeX, float *pSizeZ);

	/** Returns the 4 corner positions of the Play Area (formerly named Soft Bounds) from the working copy.
	* Corners are in clockwise order.
	* Tracking space center (0,0,0) is the center of the Play Area.
	* It's a rectangle.
	* 2 sides are parallel to the X axis and 2 sides are parallel to the Z axis.
	* Height of every corner is 0Y (on the floor). **/
	INTERFACE_FUNC(bool, GetWorkingPlayAreaRect, vr::HmdQuad_t *rect);

	/** Returns the number of Quads if the buffer points to null. Otherwise it returns Quads
	* into the buffer up to the max specified from the working copy. */
	INTERFACE_FUNC(bool, GetWorkingCollisionBoundsInfo, vr::HmdQuad_t *pQuadsBuffer, uint32_t* punQuadsCount);

	/** Returns the number of Quads if the buffer points to null. Otherwise it returns Quads
	* into the buffer up to the max specified. */
	INTERFACE_FUNC(bool, GetLiveCollisionBoundsInfo, vr::HmdQuad_t *pQuadsBuffer, uint32_t* punQuadsCount);

	/** Returns the preferred seated position from the working copy. */
	INTERFACE_FUNC(bool, GetWorkingSeatedZeroPoseToRawTrackingPose, vr::HmdMatrix34_t *pmatSeatedZeroPoseToRawTrackingPose);

	/** Returns the standing origin from the working copy. */
	INTERFACE_FUNC(bool, GetWorkingStandingZeroPoseToRawTrackingPose, vr::HmdMatrix34_t *pmatStandingZeroPoseToRawTrackingPose);

	/** Sets the Play Area in the working copy. */
	INTERFACE_FUNC(void, SetWorkingPlayAreaSize, float sizeX, float sizeZ);

	/** Sets the Collision Bounds in the working copy. */
	INTERFACE_FUNC(void, SetWorkingCollisionBoundsInfo, vr::HmdQuad_t *pQuadsBuffer, uint32_t unQuadsCount);

	/** Sets the preferred seated position in the working copy. */
	INTERFACE_FUNC(void, SetWorkingSeatedZeroPoseToRawTrackingPose, const vr::HmdMatrix34_t *pMatSeatedZeroPoseToRawTrackingPose);

	/** Sets the preferred standing position in the working copy. */
	INTERFACE_FUNC(void, SetWorkingStandingZeroPoseToRawTrackingPose, const vr::HmdMatrix34_t *pMatStandingZeroPoseToRawTrackingPose);

	/** Tear everything down and reload it from the file on disk */
	INTERFACE_FUNC(void, ReloadFromDisk, vr::IVRChaperoneSetup_005::EChaperoneConfigFile configFile);

	/** Returns the preferred seated position. */
	INTERFACE_FUNC(bool, GetLiveSeatedZeroPoseToRawTrackingPose, vr::HmdMatrix34_t *pmatSeatedZeroPoseToRawTrackingPose);

	INTERFACE_FUNC(void, SetWorkingCollisionBoundsTagsInfo, uint8_t *pTagsBuffer, uint32_t unTagCount);
	INTERFACE_FUNC(bool, GetLiveCollisionBoundsTagsInfo, uint8_t *pTagsBuffer, uint32_t *punTagCount);

	INTERFACE_FUNC(bool, SetWorkingPhysicalBoundsInfo, vr::HmdQuad_t *pQuadsBuffer, uint32_t unQuadsCount);
	INTERFACE_FUNC(bool, GetLivePhysicalBoundsInfo, vr::HmdQuad_t *pQuadsBuffer, uint32_t* punQuadsCount);

	INTERFACE_FUNC(bool, ExportLiveToBuffer, char *pBuffer, uint32_t *pnBufferLength);
	INTERFACE_FUNC(bool, ImportFromBufferToWorking, const char *pBuffer, uint32_t nImportFlags);
};
