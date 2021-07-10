#pragma once
#include "OculusDevice.h"

// "using OculusDevice::GetPose;" isn't recognised by MSVC for some reason, so pragma it out
#pragma warning(push)
#pragma warning(disable: 4250)
class OculusHMD : public OculusDevice, public IHMD {
public:
	OculusHMD(OculusBackend *backend);
	~OculusHMD() override;

	// delete the default ctors
	OculusHMD() = delete;
	OculusHMD(OculusHMD&) = delete;

	virtual bool IsConnected() override;

	using OculusDevice::GetPose;

	virtual void GetRecommendedRenderTargetSize(uint32_t * width, uint32_t * height) override;

	// from BaseSystem

	/** The projection matrix for the specified eye */
	virtual vr::HmdMatrix44_t GetProjectionMatrix(vr::EVREye eEye, float fNearZ, float fFarZ) override;
	virtual vr::HmdMatrix44_t GetProjectionMatrix(vr::EVREye eEye, float fNearZ, float fFarZ, EGraphicsAPIConvention convention) override;

	/** The components necessary to build your own projection matrix in case your
	* application is doing something fancy like infinite Z */
	virtual void GetProjectionRaw(vr::EVREye eEye, float *pfLeft, float *pfRight, float *pfTop, float *pfBottom) override;

	/** Gets the result of the distortion function for the specified eye and input UVs. UVs go from 0,0 in
	* the upper left of that eye's viewport and 1,1 in the lower right of that eye's viewport.
	* Returns true for success. Otherwise, returns false, and distortion coordinates are not suitable. */
	virtual bool ComputeDistortion(vr::EVREye eEye, float fU, float fV, vr::DistortionCoordinates_t *pDistortionCoordinates) override;

	/** Returns the transform from eye space to the head space. Eye space is the per-eye flavor of head
	* space that provides stereo disparity. Instead of Model * View * Projection the sequence is Model * View * Eye^-1 * Projection.
	* Normally View and Eye^-1 will be multiplied together and treated as View in your application.
	*/
	virtual vr::HmdMatrix34_t GetEyeToHeadTransform(vr::EVREye eEye) override;

	/** Returns the number of elapsed seconds since the last recorded vsync event. This
	*	will come from a vsync timer event in the timer if possible or from the application-reported
	*   time if that is not available. If no vsync times are available the function will
	*   return zero for vsync time and frame counter and return false from the method. */
	virtual bool GetTimeSinceLastVsync(float *pfSecondsSinceLastVsync, uint64_t *pulFrameCounter) override;

	/** Returns the hidden area mesh for the current HMD. The pixels covered by this mesh will never be seen by the user after the lens distortion is
	* applied based on visibility to the panels. If this HMD does not have a hidden area mesh, the vertex data and count will be NULL and 0 respectively.
	* This mesh is meant to be rendered into the stencil buffer (or into the depth buffer setting nearz) before rendering each eye's view.
	* This will improve performance by letting the GPU early-reject pixels the user will never see before running the pixel shader.
	* NOTE: Render this mesh with backface culling disabled since the winding order of the vertices can be different per-HMD or per-eye.
	* Setting the bInverse argument to true will produce the visible area mesh that is commonly used in place of full-screen quads.
	* The visible area mesh covers all of the pixels the hidden area mesh does not cover.
	* Setting the bLineLoop argument will return a line loop of vertices in HiddenAreaMesh_t->pVertexData with
	* HiddenAreaMesh_t->unTriangleCount set to the number of vertices.
	*/
	virtual vr::HiddenAreaMesh_t GetHiddenAreaMesh(vr::EVREye eEye, vr::EHiddenAreaMeshType) override;

	// properties
	virtual bool GetBoolTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError *pErrorL) override;
	virtual float GetFloatTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError *pErrorL) override;
	virtual uint64_t GetUint64TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError *pErrorL) override;
	virtual uint32_t GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, char *pchValue,
		uint32_t unBufferSize, vr::ETrackedPropertyError *pErrorL) override;

protected:
	virtual ovrPoseStatef GetOculusPose(const ovrTrackingState &trackingState) override;

	virtual ovrPosef GetOffset() override;

	vr::HiddenAreaMesh_t hiddenAreaMeshes[2] = {nullptr};
};
#pragma warning(pop)
