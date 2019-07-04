#pragma once
#include "BaseCommon.h"

enum OOVR_EChaperoneConfigFile {
	EChaperoneConfigFile_Live = 1,		// The live chaperone config, used by most applications and games
	EChaperoneConfigFile_Temp = 2,		// The temporary chaperone config, used to live-preview collision bounds in room setup
};

enum OOVR_EChaperoneImportFlags {
	EChaperoneImport_BoundsOnly = 0x0001,
};

class BaseChaperoneSetup {
public:
	typedef OOVR_EChaperoneConfigFile EChaperoneConfigFile;
	typedef OOVR_EChaperoneImportFlags EChaperoneConfigFlags;

	/** Saves the current working copy to disk */
	bool CommitWorkingCopy(EChaperoneConfigFile configFile);

	/** Reverts the working copy to match the live chaperone calibration.
	* To modify existing data this MUST be do WHILE getting a non-error ChaperoneCalibrationStatus.
	* Only after this should you do gets and sets on the existing data. */
	void RevertWorkingCopy();

	/** Returns the width and depth of the Play Area (formerly named Soft Bounds) in X and Z from the working copy.
	* Tracking space center (0,0,0) is the center of the Play Area. */
	bool GetWorkingPlayAreaSize(float *pSizeX, float *pSizeZ);

	/** Returns the 4 corner positions of the Play Area (formerly named Soft Bounds) from the working copy.
	* Corners are in clockwise order.
	* Tracking space center (0,0,0) is the center of the Play Area.
	* It's a rectangle.
	* 2 sides are parallel to the X axis and 2 sides are parallel to the Z axis.
	* Height of every corner is 0Y (on the floor). **/
	bool GetWorkingPlayAreaRect(vr::HmdQuad_t *rect);

	/** Returns the number of Quads if the buffer points to null. Otherwise it returns Quads
	* into the buffer up to the max specified from the working copy. */
	bool GetWorkingCollisionBoundsInfo(VR_OUT_ARRAY_COUNT(punQuadsCount) vr::HmdQuad_t *pQuadsBuffer, uint32_t* punQuadsCount);

	/** Returns the number of Quads if the buffer points to null. Otherwise it returns Quads
	* into the buffer up to the max specified. */
	bool GetLiveCollisionBoundsInfo(VR_OUT_ARRAY_COUNT(punQuadsCount) vr::HmdQuad_t *pQuadsBuffer, uint32_t* punQuadsCount);

	/** Returns the preferred seated position from the working copy. */
	bool GetWorkingSeatedZeroPoseToRawTrackingPose(vr::HmdMatrix34_t *pmatSeatedZeroPoseToRawTrackingPose);

	/** Returns the standing origin from the working copy. */
	bool GetWorkingStandingZeroPoseToRawTrackingPose(vr::HmdMatrix34_t *pmatStandingZeroPoseToRawTrackingPose);

	/** Sets the Play Area in the working copy. */
	void SetWorkingPlayAreaSize(float sizeX, float sizeZ);

	/** Sets the Collision Bounds in the working copy. */
	void SetWorkingCollisionBoundsInfo(VR_ARRAY_COUNT(unQuadsCount) vr::HmdQuad_t *pQuadsBuffer, uint32_t unQuadsCount);

	/** Sets the preferred seated position in the working copy. */
	void SetWorkingSeatedZeroPoseToRawTrackingPose(const vr::HmdMatrix34_t *pMatSeatedZeroPoseToRawTrackingPose);

	/** Sets the preferred standing position in the working copy. */
	void SetWorkingStandingZeroPoseToRawTrackingPose(const vr::HmdMatrix34_t *pMatStandingZeroPoseToRawTrackingPose);

	/** Tear everything down and reload it from the file on disk */
	void ReloadFromDisk(EChaperoneConfigFile configFile);

	/** Returns the preferred seated position. */
	bool GetLiveSeatedZeroPoseToRawTrackingPose(vr::HmdMatrix34_t *pmatSeatedZeroPoseToRawTrackingPose);

	void SetWorkingCollisionBoundsTagsInfo(VR_ARRAY_COUNT(unTagCount) uint8_t *pTagsBuffer, uint32_t unTagCount);
	bool GetLiveCollisionBoundsTagsInfo(VR_OUT_ARRAY_COUNT(punTagCount) uint8_t *pTagsBuffer, uint32_t *punTagCount);

	bool SetWorkingPhysicalBoundsInfo(VR_ARRAY_COUNT(unQuadsCount) vr::HmdQuad_t *pQuadsBuffer, uint32_t unQuadsCount);
	bool GetLivePhysicalBoundsInfo(VR_OUT_ARRAY_COUNT(punQuadsCount) vr::HmdQuad_t *pQuadsBuffer, uint32_t* punQuadsCount);

	bool ExportLiveToBuffer(VR_OUT_STRING() char *pBuffer, uint32_t *pnBufferLength);
	bool ImportFromBufferToWorking(const char *pBuffer, uint32_t nImportFlags);

	/** Sets the Collision Bounds in the working copy. */
	void SetWorkingPerimeter(VR_ARRAY_COUNT(unPointCount) vr::HmdVector2_t *pPointBuffer, uint32_t unPointCount);

	/** Shows the chaperone data in the working set to preview in the compositor.*/
	void ShowWorkingSetPreview();

	/** Hides the chaperone data in the working set to preview in the compositor (if it was visible).*/
	void HideWorkingSetPreview();

	/** Fire an event that the tracking system can use to know room setup is about to begin. This lets the tracking
	 * system make any last minute adjustments that should be incorporated into the new setup.  If the user is adjusting
	 * live in HMD using a tweak tool, keep in mind that calling this might cause the user to see the room jump. */
	void RoomSetupStarting();
};
