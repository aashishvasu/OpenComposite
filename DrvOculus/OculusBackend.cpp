#include "DrvOculusCommon.h"
#include "OculusBackend.h"

// TODO move into OCOVR
#include "../OpenOVR/stdafx.h"
#include "../OpenOVR/libovr_wrapper.h"
#include "../OpenOVR/Reimpl/static_bases.gen.h"
#include "../OpenOVR/convert.h"
#include "../OpenOVR/Reimpl/BaseSystem.h"
#include "../OpenOVR/Reimpl/BaseCompositor.h"

#include "Extras/OVR_Math.h"
using namespace OVR;

using namespace vr;

OculusBackend::OculusBackend() {
	memset(&trackingState, 0, sizeof(ovrTrackingState));

	hmd = new OculusHMD(this);
	leftHand = new OculusControllerDevice(this, EOculusTrackedObject::LTouch);
	rightHand = new OculusControllerDevice(this, EOculusTrackedObject::RTouch);
	trackedObject0 = new OculusControllerDevice(this, EOculusTrackedObject::Object0);

	// setup the device indexes
	for (vr::TrackedDeviceIndex_t i = 0; i < vr::k_unMaxTrackedDeviceCount; i++) {
		OculusDevice *dev = GetDeviceOculus(i);

		if (dev)
			dev->InitialiseDevice(i);
	}
}

OculusBackend::~OculusBackend() {
	for (int eye = 0; eye < 2; eye++) {
		if (compositors[eye])
			delete compositors[eye];
	}

	if (mirrorTexture) {
		DestroyOculusMirrorTexture();
	}
}

void OculusBackend::GetDeviceToAbsoluteTrackingPose(
	vr::ETrackingUniverseOrigin toOrigin,
	float predictedSecondsToPhotonsFromNow,
	vr::TrackedDevicePose_t * poseArray,
	uint32_t poseArrayCount) {

	ovrTrackingState trackingState = { 0 };

	if (predictedSecondsToPhotonsFromNow == 0) {
		trackingState = ovr_GetTrackingState(*ovr::session, 0 /* Most recent */, ovrFalse);
	}
	else {
		trackingState = ovr_GetTrackingState(*ovr::session, ovr_GetTimeInSeconds() + predictedSecondsToPhotonsFromNow, ovrFalse);
	}

	for (uint32_t i = 0; i < poseArrayCount; i++) {
		OculusDevice* dev = GetDeviceOculus(i);

		if(dev) {
			dev->GetPose(toOrigin, &poseArray[i], trackingState);
		} else {
			poseArray[i] = BackendManager::InvalidPose();
		}
	}

}

bool OculusBackend::GetFrameTiming(OOVR_Compositor_FrameTiming * pTiming, uint32_t unFramesAgo) {
	//if (pTiming->m_nSize != sizeof(OOVR_Compositor_FrameTiming)) {
	//	STUBBED();
	//}

	// TODO implement unFramesAgo

	ovrPerfStats stats;
	OOVR_FAILED_OVR_ABORT(ovr_GetPerfStats(*ovr::session, &stats));
	const ovrPerfStatsPerCompositorFrame &frame = stats.FrameStats[0];

	memset(pTiming, 0, sizeof(OOVR_Compositor_FrameTiming));

	pTiming->m_nSize = sizeof(OOVR_Compositor_FrameTiming); // Set to sizeof( Compositor_FrameTiming ) // TODO in methods calling this
	pTiming->m_nFrameIndex = frame.AppFrameIndex; // TODO is this per submitted frame or per HMD frame?
	pTiming->m_nNumFramePresents = 1; // TODO
	pTiming->m_nNumMisPresented = 0; // TODO
	pTiming->m_nNumDroppedFrames = 0; // TODO
	pTiming->m_nReprojectionFlags = 0;

	/** Absolute time reference for comparing frames.  This aligns with the vsync that running start is relative to. */
	// Note: OVR's method has no guarantees about aligning to vsync
	pTiming->m_flSystemTimeInSeconds = ovr_GetTimeInSeconds();

	/** These times may include work from other processes due to OS scheduling.
	* The fewer packets of work these are broken up into, the less likely this will happen.
	* GPU work can be broken up by calling Flush.  This can sometimes be useful to get the GPU started
	* processing that work earlier in the frame. */

	// time spent rendering the scene (gpu work submitted between WaitGetPoses and second Submit)
	// TODO this should be easy to time, using ovr_GetTimeInSeconds and storing
	//  that when WaitGetPoses (or PostPresentHandoff) and Submit are called, and calculating the difference
	pTiming->m_flPreSubmitGpuMs = 0;

	// additional time spent rendering by application (e.g. companion window)
	// AFAIK this is similar to m_flPreSubmitGpuMs, it's the time between PostPresentHandoff and WaitGetPoses
	// Probably not as important though
	pTiming->m_flPostSubmitGpuMs = 0;

	// time between work submitted immediately after present (ideally vsync) until the end of compositor submitted work
	// TODO CompositorCpuStartToGpuEndElapsedTime might be -1 if it's unavailable, handle that
	pTiming->m_flTotalRenderGpuMs = (frame.AppGpuElapsedTime + frame.CompositorCpuStartToGpuEndElapsedTime) / 1000;

	// time spend performing distortion correction, rendering chaperone, overlays, etc.
	pTiming->m_flCompositorRenderGpuMs = frame.CompositorGpuElapsedTime / 1000;

	// time spent on cpu submitting the above work for this frame
	// FIXME afaik CompositorCpuElapsedTime includes a bunch of other stuff too
	pTiming->m_flCompositorRenderCpuMs = frame.CompositorCpuElapsedTime / 1000;

	// time spent waiting for running start (application could have used this much more time)
	// TODO but probably not too important. I would imagine this is used primaraly for debugging.
	pTiming->m_flCompositorIdleCpuMs = 0;

	/** Miscellaneous measured intervals. */

	// time between calls to WaitGetPoses
	// TODO this should be easy to time ourselves using ovr_GetTimeInSeconds
	pTiming->m_flClientFrameIntervalMs = 0;

	// time blocked on call to present (usually 0.0, but can go long)
	// AFAIK LibOVR doesn't give us this information, but it's probably unimportant
	pTiming->m_flPresentCallCpuMs = 0;

	// time spent spin-waiting for frame index to change (not near-zero indicates wait object failure)
	// AFAIK LibOVR doesn't give us this information, though it sounds like this is again a debugging aid.
	pTiming->m_flWaitForPresentCpuMs;

	// time spent in IVRCompositor::Submit (not near-zero indicates driver issue)
	// We *could* time this, but I think it's unlikely there's any need to.
	//  This also depends on splitting up our SubmitFrame call into the three different calls and
	//  getting the wait into WaitGetPoses
	pTiming->m_flSubmitFrameMs;

	/** The following are all relative to this frame's SystemTimeInSeconds */
	// TODO these should be trivial to implement, just call ovr_GetTimeInSeconds at the right time
	/*
	pTiming->m_flWaitGetPosesCalledMs;
	pTiming->m_flNewPosesReadyMs;
	pTiming->m_flNewFrameReadyMs;
	pTiming->m_flCompositorUpdateStartMs;
	pTiming->m_flCompositorUpdateEndMs;
	pTiming->m_flCompositorRenderStartMs;
	*/

	// pose used by app to render this frame
	ETrackingUniverseOrigin origin = GetUnsafeBaseSystem()->_GetTrackingOrigin();
	GetPrimaryHMD()->GetPose(origin, &pTiming->m_HmdPose, ETrackingStateType::TrackingStateType_Rendering);

	return true;
}

ITrackedDevice* OculusBackend::GetDevice(vr::TrackedDeviceIndex_t index) {
	return GetDeviceOculus(index);
}

OculusDevice* OculusBackend::GetDeviceOculus(vr::TrackedDeviceIndex_t index) {
	OculusDevice *dev = nullptr;
	switch (index) {
	case 0:
		dev = hmd;
		break;
	case 1:
		dev = leftHand;
		break;
	case 2:
		dev = rightHand;
		break;
	case 3:
		dev = trackedObject0;
		break;
	}

	if (dev && !dev->IsConnected())
		return nullptr;

	return dev;
}

IHMD* OculusBackend::GetPrimaryHMD() {
	return hmd;
}
