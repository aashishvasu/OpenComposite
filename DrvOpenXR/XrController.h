#pragma once

#include "XrTrackedDevice.h"

class XrController : public XrTrackedDevice {
public:
	// properties
	virtual bool GetBoolTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pErrorL) override;
	virtual int32_t GetInt32TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pErrorL) override;
	virtual uint64_t GetUint64TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pErrorL) override;
	virtual uint32_t GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, char* pchValue,
	    uint32_t unBufferSize, vr::ETrackedPropertyError* pErrorL) override;
};
