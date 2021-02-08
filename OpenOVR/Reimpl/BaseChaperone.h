#pragma once
#include "BaseCommon.h"

/** HIGH LEVEL TRACKING SPACE ASSUMPTIONS:
* 0,0,0 is the preferred standing area center.
* 0Y is the floor height.
* -Z is the preferred forward facing direction. */
class BaseChaperone {
public:
	enum BaseChaperoneCalibrationState : int;

	/** Get the current state of Chaperone calibration. This state can change at any time during a session due to physical base station changes. **/
	BaseChaperoneCalibrationState GetCalibrationState();

	/** Returns the width and depth of the Play Area (formerly named Soft Bounds) in X and Z.
	* Tracking space center (0,0,0) is the center of the Play Area. **/
	bool GetPlayAreaSize(float *pSizeX, float *pSizeZ);

	/** Returns the 4 corner positions of the Play Area (formerly named Soft Bounds).
	* Corners are in counter-clockwise order.
	* Standing center (0,0,0) is the center of the Play Area.
	* It's a rectangle.
	* 2 sides are parallel to the X axis and 2 sides are parallel to the Z axis.
	* Height of every corner is 0Y (on the floor). **/
	bool GetPlayAreaRect(vr::HmdQuad_t *rect);

	/** Reload Chaperone data from the .vrchap file on disk. */
	void ReloadInfo(void);

	/** Optionally give the chaperone system a hit about the color and brightness in the scene **/
	void SetSceneColor(vr::HmdColor_t color);

	/** Get the current chaperone bounds draw color and brightness **/
	void GetBoundsColor(vr::HmdColor_t *pOutputColorArray, int nNumOutputColors, float flCollisionBoundsFadeDistance, vr::HmdColor_t *pOutputCameraColor);

	/** Determine whether the bounds are showing right now **/
	bool AreBoundsVisible();

	/** Force the bounds to show, mostly for utilities **/
	void ForceBoundsVisible(bool bForce);

	/** Sets the zero pose for the given tracker coordinate system to the current position and yaw of the HMD. After
	 * ResetZeroPose all GetDeviceToAbsoluteTrackingPose calls as the origin will be relative to this new zero pose.
	 * The new zero coordinate system will not change the fact that the Y axis is up in the real world, so the next
	 * pose returned from GetDeviceToAbsoluteTrackingPose after a call to ResetZeroPose may not be exactly an
	 * identity matrix.
	 *
	 * NOTE: This function overrides the user's previously saved zero pose and should only be called as the result of a user action.
	 * Users are also able to set their zero pose via the OpenVR Dashboard.
	 *
	 * NOTE: This used to be called BaseSystem::ResetSeatedZeroPose but was moved in OpenVR 1.13.10
	 **/
	virtual void ResetZeroPose(vr::ETrackingUniverseOrigin eTrackingUniverseOrigin);

	enum BaseChaperoneCalibrationState : int {
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

private:
	bool GetMinMaxPoints(vr::HmdVector3_t &minPoint, vr::HmdVector3_t &maxPoint);
};
