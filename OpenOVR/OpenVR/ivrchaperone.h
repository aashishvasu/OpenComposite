#pragma once

#include "vrtypes.h"
#include "vrannotation.h"

// ivrchaperone.h
namespace vr {

#pragma pack( push, 8 )

	enum ChaperoneCalibrationState {
		// OK!
		ChaperoneCalibrationState_OK = 1,									// Chaperone is fully calibrated and working correctly

																			// Warnings
																			ChaperoneCalibrationState_Warning = 100,
																			ChaperoneCalibrationState_Warning_BaseStationMayHaveMoved = 101,	// A base station thinks that it might have moved
																			ChaperoneCalibrationState_Warning_BaseStationRemoved = 102,			// There are less base stations than when calibrated
																			ChaperoneCalibrationState_Warning_SeatedBoundsInvalid = 103,		// Seated bounds haven't been calibrated for the current tracking center

																																				// Errors
																																				ChaperoneCalibrationState_Error = 200,								// The UniverseID is invalid
																																				ChaperoneCalibrationState_Error_BaseStationUninitialized = 201,		// Tracking center hasn't be calibrated for at least one of the base stations
																																				ChaperoneCalibrationState_Error_BaseStationConflict = 202,			// Tracking center is calibrated, but base stations disagree on the tracking space
																																				ChaperoneCalibrationState_Error_PlayAreaInvalid = 203,				// Play Area hasn't been calibrated for the current tracking center
																																				ChaperoneCalibrationState_Error_CollisionBoundsInvalid = 204,		// Collision Bounds haven't been calibrated for the current tracking center
	};


	/** HIGH LEVEL TRACKING SPACE ASSUMPTIONS:
	* 0,0,0 is the preferred standing area center.
	* 0Y is the floor height.
	* -Z is the preferred forward facing direction. */
	class IVRChaperone {
	public:

		/** Get the current state of Chaperone calibration. This state can change at any time during a session due to physical base station changes. **/
		virtual ChaperoneCalibrationState GetCalibrationState() = 0;

		/** Returns the width and depth of the Play Area (formerly named Soft Bounds) in X and Z.
		* Tracking space center (0,0,0) is the center of the Play Area. **/
		virtual bool GetPlayAreaSize(float *pSizeX, float *pSizeZ) = 0;

		/** Returns the 4 corner positions of the Play Area (formerly named Soft Bounds).
		* Corners are in counter-clockwise order.
		* Standing center (0,0,0) is the center of the Play Area.
		* It's a rectangle.
		* 2 sides are parallel to the X axis and 2 sides are parallel to the Z axis.
		* Height of every corner is 0Y (on the floor). **/
		virtual bool GetPlayAreaRect(HmdQuad_t *rect) = 0;

		/** Reload Chaperone data from the .vrchap file on disk. */
		virtual void ReloadInfo(void) = 0;

		/** Optionally give the chaperone system a hit about the color and brightness in the scene **/
		virtual void SetSceneColor(HmdColor_t color) = 0;

		/** Get the current chaperone bounds draw color and brightness **/
		virtual void GetBoundsColor(HmdColor_t *pOutputColorArray, int nNumOutputColors, float flCollisionBoundsFadeDistance, HmdColor_t *pOutputCameraColor) = 0;

		/** Determine whether the bounds are showing right now **/
		virtual bool AreBoundsVisible() = 0;

		/** Force the bounds to show, mostly for utilities **/
		virtual void ForceBoundsVisible(bool bForce) = 0;
	};

	static const char * const IVRChaperone_Version = "IVRChaperone_003";

#pragma pack( pop )

}

// ivrchaperonesetup.h
namespace vr {

	enum EChaperoneConfigFile {
		EChaperoneConfigFile_Live = 1,		// The live chaperone config, used by most applications and games
		EChaperoneConfigFile_Temp = 2,		// The temporary chaperone config, used to live-preview collision bounds in room setup
	};

	enum EChaperoneImportFlags {
		EChaperoneImport_BoundsOnly = 0x0001,
	};

	/** Manages the working copy of the chaperone info. By default this will be the same as the
	* live copy. Any changes made with this interface will stay in the working copy until
	* CommitWorkingCopy() is called, at which point the working copy and the live copy will be
	* the same again. */
	class IVRChaperoneSetup {
	public:

		/** Saves the current working copy to disk */
		virtual bool CommitWorkingCopy(EChaperoneConfigFile configFile) = 0;

		/** Reverts the working copy to match the live chaperone calibration.
		* To modify existing data this MUST be do WHILE getting a non-error ChaperoneCalibrationStatus.
		* Only after this should you do gets and sets on the existing data. */
		virtual void RevertWorkingCopy() = 0;

		/** Returns the width and depth of the Play Area (formerly named Soft Bounds) in X and Z from the working copy.
		* Tracking space center (0,0,0) is the center of the Play Area. */
		virtual bool GetWorkingPlayAreaSize(float *pSizeX, float *pSizeZ) = 0;

		/** Returns the 4 corner positions of the Play Area (formerly named Soft Bounds) from the working copy.
		* Corners are in clockwise order.
		* Tracking space center (0,0,0) is the center of the Play Area.
		* It's a rectangle.
		* 2 sides are parallel to the X axis and 2 sides are parallel to the Z axis.
		* Height of every corner is 0Y (on the floor). **/
		virtual bool GetWorkingPlayAreaRect(HmdQuad_t *rect) = 0;

		/** Returns the number of Quads if the buffer points to null. Otherwise it returns Quads
		* into the buffer up to the max specified from the working copy. */
		virtual bool GetWorkingCollisionBoundsInfo(VR_OUT_ARRAY_COUNT(punQuadsCount) HmdQuad_t *pQuadsBuffer, uint32_t* punQuadsCount) = 0;

		/** Returns the number of Quads if the buffer points to null. Otherwise it returns Quads
		* into the buffer up to the max specified. */
		virtual bool GetLiveCollisionBoundsInfo(VR_OUT_ARRAY_COUNT(punQuadsCount) HmdQuad_t *pQuadsBuffer, uint32_t* punQuadsCount) = 0;

		/** Returns the preferred seated position from the working copy. */
		virtual bool GetWorkingSeatedZeroPoseToRawTrackingPose(HmdMatrix34_t *pmatSeatedZeroPoseToRawTrackingPose) = 0;

		/** Returns the standing origin from the working copy. */
		virtual bool GetWorkingStandingZeroPoseToRawTrackingPose(HmdMatrix34_t *pmatStandingZeroPoseToRawTrackingPose) = 0;

		/** Sets the Play Area in the working copy. */
		virtual void SetWorkingPlayAreaSize(float sizeX, float sizeZ) = 0;

		/** Sets the Collision Bounds in the working copy. */
		virtual void SetWorkingCollisionBoundsInfo(VR_ARRAY_COUNT(unQuadsCount) HmdQuad_t *pQuadsBuffer, uint32_t unQuadsCount) = 0;

		/** Sets the preferred seated position in the working copy. */
		virtual void SetWorkingSeatedZeroPoseToRawTrackingPose(const HmdMatrix34_t *pMatSeatedZeroPoseToRawTrackingPose) = 0;

		/** Sets the preferred standing position in the working copy. */
		virtual void SetWorkingStandingZeroPoseToRawTrackingPose(const HmdMatrix34_t *pMatStandingZeroPoseToRawTrackingPose) = 0;

		/** Tear everything down and reload it from the file on disk */
		virtual void ReloadFromDisk(EChaperoneConfigFile configFile) = 0;

		/** Returns the preferred seated position. */
		virtual bool GetLiveSeatedZeroPoseToRawTrackingPose(HmdMatrix34_t *pmatSeatedZeroPoseToRawTrackingPose) = 0;

		virtual void SetWorkingCollisionBoundsTagsInfo(VR_ARRAY_COUNT(unTagCount) uint8_t *pTagsBuffer, uint32_t unTagCount) = 0;
		virtual bool GetLiveCollisionBoundsTagsInfo(VR_OUT_ARRAY_COUNT(punTagCount) uint8_t *pTagsBuffer, uint32_t *punTagCount) = 0;

		virtual bool SetWorkingPhysicalBoundsInfo(VR_ARRAY_COUNT(unQuadsCount) HmdQuad_t *pQuadsBuffer, uint32_t unQuadsCount) = 0;
		virtual bool GetLivePhysicalBoundsInfo(VR_OUT_ARRAY_COUNT(punQuadsCount) HmdQuad_t *pQuadsBuffer, uint32_t* punQuadsCount) = 0;

		virtual bool ExportLiveToBuffer(VR_OUT_STRING() char *pBuffer, uint32_t *pnBufferLength) = 0;
		virtual bool ImportFromBufferToWorking(const char *pBuffer, uint32_t nImportFlags) = 0;
	};

	static const char * const IVRChaperoneSetup_Version = "IVRChaperoneSetup_005";


}
