#include "stdafx.h"
#define GENFILE
#include "BaseCommon.h"

GEN_INTERFACE("ServerDriverHost", "005", DRIVER)

#include "OpenVR/interfaces/driver_IVRServerDriverHost_005.h"
using ITrackedDeviceServerDriver005 = vr::driver_ITrackedDeviceServerDriver_005::ITrackedDeviceServerDriver;
using DriverPose_t005 = vr::driver_ITrackedDeviceServerDriver_005::DriverPose_t;

#include "GVRServerDriverHost.gen.h"

// Version-indepentant proxy
// TODO auto-generate these
class OCTrackedDeviceDriver_005 : public OCTrackedDeviceDriver {
public:
	OCTrackedDeviceDriver_005(const char *serialNumber, vr::ETrackedDeviceClass deviceClass, ITrackedDeviceServerDriver005 *driver)
		: OCTrackedDeviceDriver(serialNumber, deviceClass), driver(driver) {
	}

	virtual vr::EVRInitError Activate(uint32_t unObjectId) {
		return driver->Activate(unObjectId);
	}

	virtual void Deactivate() {
		driver->Deactivate();
	}

	virtual void EnterStandby() {
		driver->EnterStandby();
	}

	virtual void *GetComponent(const char *pchComponentNameAndVersion) {
		return driver->GetComponent(pchComponentNameAndVersion);
	}

	virtual void DebugRequest(const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize) {
		driver->DebugRequest(pchRequest, pchResponseBuffer, unResponseBufferSize);
	}

	virtual OCDriverPose_t GetPose() {
		DriverPose_t005 p5 = driver->GetPose();

		OCDriverPose_t p;
		memset(&p, 0, sizeof(p));

		static_assert(sizeof(p5) == sizeof(p), "v5 and OC driver pose size mismatch");
		memcpy(&p, &p5, sizeof(p5));

		return p;
	}

private:
	ITrackedDeviceServerDriver005 *driver;
};

bool CVRServerDriverHost_005::TrackedDeviceAdded(const char* serialNumber, vr::ETrackedDeviceClass deviceClass,
	ITrackedDeviceServerDriver005* pDriver) {

	// TODO implement
	OCTrackedDeviceDriver *driver = new OCTrackedDeviceDriver_005(serialNumber, deviceClass, pDriver);
	return base->TrackedDeviceAdded(driver);
}

void CVRServerDriverHost_005::TrackedDevicePoseUpdated(uint32_t unWhichDevice, const DriverPose_t005& newPose, uint32_t unPoseStructSize) {
	base->TrackedDevicePoseUpdated(unWhichDevice, (OCDriverPose_t&)newPose, unPoseStructSize);
}
