#include "stdafx.h"
#include "DriverManager.h"

#include "steamvr_abi.h"

using namespace vr;

// context
OCDriverContext::OCDriverContext(vr::IServerTrackedDeviceProvider *provider) : provider(provider) {
}

void * OCDriverContext::GetGenericInterface(const char * pchInterfaceVersion, EVRInitError * peError) {
	return VR_GetGenericInterface(pchInterfaceVersion, peError);
}

DriverHandle_t OCDriverContext::GetDriverHandle() {
	OOVR_ABORT("GetDriverHandle not yet implemented");
}

// tracked device

OCTrackedDeviceDriver::OCTrackedDeviceDriver(const char * pchDeviceSerialNumber, vr::ETrackedDeviceClass eDeviceClass)
	: serialNumber(pchDeviceSerialNumber), deviceClass(eDeviceClass) {
}

// driver manager
std::unique_ptr<DriverManager> DriverManager::instance;

DriverManager& DriverManager::Instance() {
	if (!instance)
		instance.reset(new DriverManager());

	return *instance;
}

DriverManager& DriverManager::CheckInstance() {
	if (!instance)
		OOVR_ABORT("Driver manager not initialised!");

	return *instance;
}

void DriverManager::Reset() {
	instance.reset();
}

DriverManager::DriverManager() {
}

DriverManager::~DriverManager() {
	for (OCTrackedDeviceDriver *obj : trackedObjects) {
		delete obj;
	}
	if (hmd)
		delete hmd;
	for (OCDriverContext *ctx : drivers) {
		delete ctx;
	}
}

void DriverManager::Register(vr::IServerTrackedDeviceProvider *provider) {
	OCDriverContext *ctx = new OCDriverContext(provider);
	vr::EVRInitError err = provider->Init(ctx);

	drivers.push_back(ctx);
}

// Forwarded from BaseServerDriverHost
bool DriverManager::TrackedDeviceAdded(OCTrackedDeviceDriver *pDriver) {
	// TODO make this configurable
	if (!hmd && pDriver->DeviceClass() == vr::ETrackedDeviceClass::TrackedDeviceClass_HMD) {
		hmd = pDriver;
		pDriver->index = 0;
	}
	else {
		trackedObjects.push_back(pDriver);

		// Take the size after adding it, to increase it by one
		// since HMD takes the zero position
		pDriver->index = (vr::TrackedDeviceIndex_t)trackedObjects.size();
	}

	pDriver->Activate();

	return true;
}
