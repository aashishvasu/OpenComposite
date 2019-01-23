#pragma once
#include <vector>
#include <memory>

// TODO get this file to split correctly, so we can use different versions of it's interfaces
#include "OpenVR/interfaces/driver_itrackeddevicedriverprovider.h"

class OCDriverContext : public vr::IVRDriverContext {
public:
	OCDriverContext(vr::IServerTrackedDeviceProvider*);

	/** Returns the requested interface. If the interface was not available it will return NULL and fill
	* out the error. */
	virtual void *GetGenericInterface(const char *pchInterfaceVersion, vr::EVRInitError *peError = nullptr);

	/** Returns the property container handle for this driver */
	virtual vr::DriverHandle_t GetDriverHandle();

private:
	vr::IServerTrackedDeviceProvider *provider;
};

// tracked device driver
struct OCDriverPose_t {
	/* Time offset of this pose, in seconds from the actual time of the pose,
	* relative to the time of the PoseUpdated() call made by the driver.
	*/
	double poseTimeOffset;

	/* Generally, the pose maintained by a driver
	* is in an inertial coordinate system different
	* from the world system of x+ right, y+ up, z+ back.
	* Also, the driver is not usually tracking the "head" position,
	* but instead an internal IMU or another reference point in the HMD.
	* The following two transforms transform positions and orientations
	* to app world space from driver world space,
	* and to HMD head space from driver local body space.
	*
	* We maintain the driver pose state in its internal coordinate system,
	* so we can do the pose prediction math without having to
	* use angular acceleration.  A driver's angular acceleration is generally not measured,
	* and is instead calculated from successive samples of angular velocity.
	* This leads to a noisy angular acceleration values, which are also
	* lagged due to the filtering required to reduce noise to an acceptable level.
	*/
	vr::HmdQuaternion_t qWorldFromDriverRotation;
	double vecWorldFromDriverTranslation[3];

	vr::HmdQuaternion_t qDriverFromHeadRotation;
	double vecDriverFromHeadTranslation[3];

	/* State of driver pose, in meters and radians. */
	/* Position of the driver tracking reference in driver world space
	* +[0] (x) is right
	* +[1] (y) is up
	* -[2] (z) is forward
	*/
	double vecPosition[3];

	/* Velocity of the pose in meters/second */
	double vecVelocity[3];

	/* Acceleration of the pose in meters/second */
	double vecAcceleration[3];

	/* Orientation of the tracker, represented as a quaternion */
	vr::HmdQuaternion_t qRotation;

	/* Angular velocity of the pose in axis-angle
	* representation. The direction is the angle of
	* rotation and the magnitude is the angle around
	* that axis in radians/second. */
	double vecAngularVelocity[3];

	/* Angular acceleration of the pose in axis-angle
	* representation. The direction is the angle of
	* rotation and the magnitude is the angle around
	* that axis in radians/second^2. */
	double vecAngularAcceleration[3];

	vr::ETrackingResult result;

	bool poseIsValid;
	bool willDriftInYaw;
	bool shouldApplyHeadModel;
	bool deviceIsConnected;
};

class OCTrackedDeviceDriver {
public:
	OCTrackedDeviceDriver(const char *pchDeviceSerialNumber, vr::ETrackedDeviceClass eDeviceClass);

	// OC getters
	std::string SerialNumber() { return serialNumber; }
	vr::ETrackedDeviceClass DeviceClass() { return deviceClass; }
	vr::TrackedDeviceIndex_t DeviceIndex() { return index; }

	// ------------------------------------
	// Management Methods
	// ------------------------------------
	/** This is called before an HMD is returned to the application. It will always be
	* called before any display or tracking methods. Memory and processor use by the
	* ITrackedDeviceServerDriver object should be kept to a minimum until it is activated.
	* The pose listener is guaranteed to be valid until Deactivate is called, but
	* should not be used after that point. */
	virtual vr::EVRInitError Activate(uint32_t unObjectId) = 0;


	virtual vr::EVRInitError Activate() {
		return Activate(DeviceIndex());
	}

	/** This is called when The VR system is switching from this Hmd being the active display
	* to another Hmd being the active display. The driver should clean whatever memory
	* and thread use it can when it is deactivated */
	virtual void Deactivate() = 0;

	/** Handles a request from the system to put this device into standby mode. What that means is defined per-device. */
	virtual void EnterStandby() = 0;

	/** Requests a component interface of the driver for device-specific functionality. The driver should return NULL
	* if the requested interface or version is not supported. */
	virtual void *GetComponent(const char *pchComponentNameAndVersion) = 0;

	/** A VR Client has made this debug request of the driver. The set of valid requests is entirely
	* up to the driver and the client to figure out, as is the format of the response. Responses that
	* exceed the length of the supplied buffer should be truncated and null terminated */
	virtual void DebugRequest(const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize) = 0;

	// ------------------------------------
	// Tracking Methods
	// ------------------------------------
	virtual OCDriverPose_t GetPose() = 0;

private:
	// OCDriverContext *driver;
	std::string serialNumber;
	vr::ETrackedDeviceClass deviceClass;
	vr::TrackedDeviceIndex_t index = ~0; // invalid value

	// Needs to set the device index
	friend class DriverManager;
};

class DriverManager {
public:
	static DriverManager& Instance();
	static DriverManager& CheckInstance();
	static void Reset();

	void Register(vr::IServerTrackedDeviceProvider*);

	// Forwarded from BaseServerDriverHost

	/**
	 * Register a physical device.
	 * Return true for success, false for an error registering it.
	 */
	bool TrackedDeviceAdded(OCTrackedDeviceDriver *pDriver);

private:
	DriverManager();

	// Required for private dtor
	friend std::default_delete<DriverManager>;
	~DriverManager();

	OCTrackedDeviceDriver *hmd = nullptr;
	std::vector<OCDriverContext*> drivers;
	std::vector<OCTrackedDeviceDriver*> trackedObjects;

	static std::unique_ptr<DriverManager> instance;
};
