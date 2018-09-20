#pragma once
#include "BaseChaperone.h"
#include "OpenVR/interfaces/IVRChaperone_003.h"

/** HIGH LEVEL TRACKING SPACE ASSUMPTIONS:
* 0,0,0 is the preferred standing area center.
* 0Y is the floor height.
* -Z is the preferred forward facing direction. */
class CVRChaperone_003 : public vr::IVRChaperone_003::IVRChaperone, public CVRCommon {
	CVR_GEN_IFACE();

private:
	BaseChaperone base;
public:

	/** Get the current state of Chaperone calibration. This state can change at any time during a session due to physical base station changes. **/
	INTERFACE_FUNC(vr::IVRChaperone_003::ChaperoneCalibrationState, GetCalibrationState, void);

	/** Returns the width and depth of the Play Area (formerly named Soft Bounds) in X and Z.
	* Tracking space center (0,0,0) is the center of the Play Area. **/
	INTERFACE_FUNC(bool, GetPlayAreaSize, float *pSizeX, float *pSizeZ);

	/** Returns the 4 corner positions of the Play Area (formerly named Soft Bounds).
	* Corners are in counter-clockwise order.
	* Standing center (0,0,0) is the center of the Play Area.
	* It's a rectangle.
	* 2 sides are parallel to the X axis and 2 sides are parallel to the Z axis.
	* Height of every corner is 0Y (on the floor). **/
	INTERFACE_FUNC(bool, GetPlayAreaRect, vr::HmdQuad_t *rect);

	/** Reload Chaperone data from the .vrchap file on disk. */
	INTERFACE_FUNC(void, ReloadInfo, void);

	/** Optionally give the chaperone system a hit about the color and brightness in the scene **/
	INTERFACE_FUNC(void, SetSceneColor, vr::HmdColor_t color);

	/** Get the current chaperone bounds draw color and brightness **/
	INTERFACE_FUNC(void, GetBoundsColor, vr::HmdColor_t *pOutputColorArray, int nNumOutputColors, float flCollisionBoundsFadeDistance, vr::HmdColor_t *pOutputCameraColor);

	/** Determine whether the bounds are showing right now **/
	INTERFACE_FUNC(bool, AreBoundsVisible);

	/** Force the bounds to show, mostly for utilities **/
	INTERFACE_FUNC(void, ForceBoundsVisible, bool bForce);
};
