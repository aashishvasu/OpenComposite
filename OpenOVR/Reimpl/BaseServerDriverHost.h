#pragma once
#include "BaseCommon.h"

// TODO get this to work properly
#include "OpenVR/interfaces/driver_IVRServerDriverHost_005.h"

#include "Drivers/DriverManager.h"

class BaseServerDriverHost {
public:

	/** Notifies the server that a tracked device has been added. If this function returns true
	* the server will call Activate on the device. If it returns false some kind of error
	* has occurred and the device will not be activated. */
	virtual bool TrackedDeviceAdded(OCTrackedDeviceDriver *driver);

	/** Notifies the server that a tracked device's pose has been updated */
	virtual void TrackedDevicePoseUpdated(uint32_t unWhichDevice, const OCDriverPose_t & newPose, uint32_t unPoseStructSize);

	/** Notifies the server that vsync has occurred on the the display attached to the device. This is
	* only permitted on devices of the HMD class. */
	virtual void VsyncEvent(double vsyncTimeOffsetSeconds);

	/** Sends a vendor specific event (VREvent_VendorSpecific_Reserved_Start..VREvent_VendorSpecific_Reserved_End */
	virtual void VendorSpecificEvent(uint32_t unWhichDevice, vr::EVREventType eventType, const vr::VREvent_Data_t & eventData, double eventTimeOffset);

	/** Returns true if SteamVR is exiting */
	virtual bool IsExiting();

	/** Returns true and fills the event with the next event on the queue if there is one. If there are no events
	* this method returns false. uncbVREvent should be the size in bytes of the VREvent_t struct */
	virtual bool PollNextEvent(vr::VREvent_t *pEvent, uint32_t uncbVREvent);

	/** Provides access to device poses for drivers.  Poses are in their "raw" tracking space which is uniquely
	* defined by each driver providing poses for its devices.  It is up to clients of this function to correlate
	* poses across different drivers.  Poses are indexed by their device id, and their associated driver and
	* other properties can be looked up via IVRProperties. */
	virtual void GetRawTrackedDevicePoses(float fPredictedSecondsFromNow, vr::TrackedDevicePose_t *pTrackedDevicePoseArray, uint32_t unTrackedDevicePoseArrayCount);

	/** Notifies the server that a tracked device's display component transforms have been updated. */
	virtual void TrackedDeviceDisplayTransformUpdated(uint32_t unWhichDevice, vr::HmdMatrix34_t eyeToHeadLeft, vr::HmdMatrix34_t eyeToHeadRight);
};
