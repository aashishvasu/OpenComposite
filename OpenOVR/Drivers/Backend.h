#pragma once
#include "../OpenOVR/custom_types.h" // TODO move this into the OpenVR tree
#include "generated/interfaces/vrtypes.h"
#include <memory>

// for OOVR_Compositor_FrameTiming
#include "../OpenOVR/Reimpl/BaseCompositor.h"

// Avoid including InteractionProfile for a single use
class InteractionProfile;

enum ETrackingStateType {
	/**
	 * Use the latest available tracking data
	 */
	TrackingStateType_Now,

	/**
	 * Use the tracking data corresponding to the upcoming frame
	 */
	TrackingStateType_Rendering,

	/**
	 * Use the tracking data corresponding to the predicted time
	 */
	TrackingStateType_Prediction,
};

/**
 * Represents a single physical device
 */
class ITrackedDevice {
public:
	virtual ~ITrackedDevice();

	enum HandType {
		HAND_LEFT,
		HAND_RIGHT,
		HAND_NONE,
	};

	virtual void GetPose(
	    vr::ETrackingUniverseOrigin origin,
	    vr::TrackedDevicePose_t* pose,
	    ETrackingStateType trackingState)
	    = 0;

	virtual void GetPose(
	    vr::ETrackingUniverseOrigin origin,
	    vr::TrackedDevicePose_t* pose,
	    ETrackingStateType trackingState,
	    double absTime)
	    = 0;

	virtual vr::ETrackedDeviceClass GetTrackedDeviceClass() = 0;

	virtual vr::ETrackedControllerRole GetControllerRole();

	/** Returns a bool property. If the device index is not valid or the property is not a bool type this function will return false. */
	virtual bool GetBoolTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pErrorL);

	/** Returns a float property. If the device index is not valid or the property is not a float type this function will return 0. */
	virtual float GetFloatTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pErrorL);

	/** Returns an int property. If the device index is not valid or the property is not a int type this function will return 0. */
	virtual int32_t GetInt32TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pErrorL);

	/** Returns a uint64 property. If the device index is not valid or the property is not a uint64 type this function will return 0. */
	virtual uint64_t GetUint64TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pErrorL);

	/** Returns a matrix property. If the device index is not valid or the property is not a matrix type, this function will return identity. */
	virtual vr::HmdMatrix34_t GetMatrix34TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pErrorL);

	/** Returns an array of one type of property. If the device index is not valid or the property is not a single value or an array of the specified type,
	 * this function will return 0. Otherwise it returns the number of bytes necessary to hold the array of properties. If unBufferSize is
	 * greater than the returned size and pBuffer is non-NULL, pBuffer is filled with the contents of array of properties. */
	virtual uint32_t GetArrayTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::PropertyTypeTag_t propType, void* pBuffer,
	    uint32_t unBufferSize, vr::ETrackedPropertyError* pError);

	/** Returns a string property. If the device index is not valid or the property is not a string type this function will
	 * return 0. Otherwise it returns the length of the number of bytes necessary to hold this string including the trailing
	 * null. Strings will always fit in buffers of k_unMaxPropertyStringSize characters. */
	virtual uint32_t GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, char* pchValue,
	    uint32_t unBufferSize, vr::ETrackedPropertyError* pErrorL);

	/** Fills the supplied struct with the current state of the controller. Returns false if the controller index
	 * is invalid. */
	virtual bool GetControllerState(vr::VRControllerState_t* pControllerState);

	/** Triggers a haptic event, running at the given frequency for and given amplitude. This will be called again to stop the action. */
	virtual int32_t TriggerHapticVibrationAction(float fFrequency, float fAmplitude);

	/**
	 * Get the hand represented by this device, if any. Used mainly for the input system.
	 */
	virtual HandType GetHand();

	/**
	 * Get the interaction profile that best represents this controller, or null if this isn't a controller or there isn't a suitable profile.
	 *
	 * This should be used as little as possible, since the interaction-related stuff should generally be kept inside the controller.
	 */
	virtual const InteractionProfile* GetInteractionProfile();

	///////
	vr::TrackedDeviceIndex_t DeviceIndex();
	virtual void InitialiseDevice(vr::TrackedDeviceIndex_t deviceIndex);

private:
	vr::TrackedDeviceIndex_t deviceIndex = vr::k_unTrackedDeviceIndexInvalid;
};

/**
 * Represents a head-mounted display
 */
class IHMD : public virtual ITrackedDevice {
public:
	virtual void GetRecommendedRenderTargetSize(uint32_t* width, uint32_t* height) = 0;

	vr::ETrackedDeviceClass GetTrackedDeviceClass() override;

	// from BaseSystem

	/** The projection matrix for the specified eye */
	virtual vr::HmdMatrix44_t GetProjectionMatrix(vr::EVREye eEye, float fNearZ, float fFarZ, EGraphicsAPIConvention convention = API_DirectX) = 0;

	/** The components necessary to build your own projection matrix in case your
	 * application is doing something fancy like infinite Z */
	virtual void GetProjectionRaw(vr::EVREye eEye, float* pfLeft, float* pfRight, float* pfTop, float* pfBottom) = 0;

	/** Gets the result of the distortion function for the specified eye and input UVs. UVs go from 0,0 in
	 * the upper left of that eye's viewport and 1,1 in the lower right of that eye's viewport.
	 * Returns true for success. Otherwise, returns false, and distortion coordinates are not suitable. */
	virtual bool ComputeDistortion(vr::EVREye eEye, float fU, float fV, vr::DistortionCoordinates_t* pDistortionCoordinates) = 0;

	/** Returns the transform from eye space to the head space. Eye space is the per-eye flavor of head
	 * space that provides stereo disparity. Instead of Model * View * Projection the sequence is Model * View * Eye^-1 * Projection.
	 * Normally View and Eye^-1 will be multiplied together and treated as View in your application.
	 */
	virtual vr::HmdMatrix34_t GetEyeToHeadTransform(vr::EVREye eEye) = 0;

	virtual float GetIPD() = 0;

	/** Returns the number of elapsed seconds since the last recorded vsync event. This
	 *	will come from a vsync timer event in the timer if possible or from the application-reported
	 *   time if that is not available. If no vsync times are available the function will
	 *   return zero for vsync time and frame counter and return false from the method. */
	virtual bool GetTimeSinceLastVsync(float* pfSecondsSinceLastVsync, uint64_t* pulFrameCounter) = 0;

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
	virtual vr::HiddenAreaMesh_t GetHiddenAreaMesh(vr::EVREye eEye, vr::EHiddenAreaMeshType type) = 0;
};

#define DECLARE_BACKEND_FUNCS(PREPEND, APPEND)                                                                                                     \
	PREPEND IHMD* GetPrimaryHMD() APPEND;                                                                                                          \
                                                                                                                                                   \
	PREPEND ITrackedDevice* GetDevice(                                                                                                             \
	    vr::TrackedDeviceIndex_t index) APPEND;                                                                                                    \
                                                                                                                                                   \
	/* Get the first (and hopefully only) device of a given hand type, or nullptr */                                                               \
	PREPEND ITrackedDevice* GetDeviceByHand(                                                                                                       \
	    ITrackedDevice::HandType hand) APPEND;                                                                                                     \
                                                                                                                                                   \
	PREPEND void GetDeviceToAbsoluteTrackingPose(                                                                                                  \
	    vr::ETrackingUniverseOrigin toOrigin,                                                                                                      \
	    float predictedSecondsToPhotonsFromNow,                                                                                                    \
	    vr::TrackedDevicePose_t* poseArray,                                                                                                        \
	    uint32_t poseArrayCount) APPEND;                                                                                                           \
                                                                                                                                                   \
	/* Submitting Frames */                                                                                                                        \
	PREPEND void WaitForTrackingData() APPEND;                                                                                                     \
                                                                                                                                                   \
	PREPEND void StoreEyeTexture(                                                                                                                  \
	    vr::EVREye eye,                                                                                                                            \
	    const vr::Texture_t* texture,                                                                                                              \
	    const vr::VRTextureBounds_t* bounds,                                                                                                       \
	    vr::EVRSubmitFlags submitFlags,                                                                                                            \
	    bool isFirstEye) APPEND;                                                                                                                   \
                                                                                                                                                   \
	PREPEND void SubmitFrames(bool showSkybox, bool postPresent) APPEND;                                                                           \
                                                                                                                                                   \
	PREPEND IBackend::openvr_enum_t SetSkyboxOverride(const vr::Texture_t* pTextures, uint32_t unTextureCount) APPEND;                             \
                                                                                                                                                   \
	PREPEND void ClearSkyboxOverride() APPEND;                                                                                                     \
                                                                                                                                                   \
	/* Misc compositor */                                                                                                                          \
                                                                                                                                                   \
	/**                                                                                                                                            \
	 * Get frame timing information to be passed to the application                                                                                \
	 *                                                                                                                                             \
	 * Returns true if successful                                                                                                                  \
	 */                                                                                                                                            \
	PREPEND bool GetFrameTiming(OOVR_Compositor_FrameTiming* pTiming, uint32_t unFramesAgo) APPEND;                                                \
                                                                                                                                                   \
	/* D3D Mirror textures */                                                                                                                      \
	/* #if defined(SUPPORT_DX) */                                                                                                                  \
	PREPEND IBackend::openvr_enum_t GetMirrorTextureD3D11(vr::EVREye eEye, void* pD3D11DeviceOrResource, void** ppD3D11ShaderResourceView) APPEND; \
	PREPEND void ReleaseMirrorTextureD3D11(void* pD3D11ShaderResourceView) APPEND;                                                                 \
	/* #endif */                                                                                                                                   \
	/** Returns the points of the Play Area. */                                                                                                    \
	PREPEND bool GetPlayAreaPoints(vr::HmdVector3_t* points, int* count) APPEND;                                                                   \
	/** Determine whether the bounds are showing right now **/                                                                                     \
	PREPEND bool AreBoundsVisible() APPEND;                                                                                                        \
	/** Set the boundaries to be visible or not (although setting this to false shouldn't affect                                                   \
	 * what happens if the player moves their hands too close and shows it that way) **/                                                           \
	PREPEND void ForceBoundsVisible(bool status) APPEND;                                                                                           \
	/** Capture OpenXR events. This should normally be done every frame, but this can be                                                           \
	 * used to poll for events more frequently than that, such as on android while suspended. */                                                   \
	PREPEND void PumpEvents() APPEND;                                                                                                              \
	/** Returns true if the runtime is focused and receiving input */                                                                              \
	PREPEND bool IsInputAvailable() APPEND;                                                                                                        \
	/** Returns true if the application graphics device is being used */                                                                           \
	PREPEND bool IsGraphicsConfigured() APPEND;                                                                                                    \
	PREPEND void OnOverlayTexture(const vr::Texture_t* texture) APPEND;

/**
 * Backend is similar in concept to the OpenVR driver API, however it is
 * much closer to the application-facing API, and thus can avoid quite a bit of
 * the potential performance issues that arise when using the OpenVR driver API when
 * proxying to another high-level API (such as LibOVR or OpenHMD).
 *
 * Basically, it's a single interface class that can do anything a OpenVR driver can do. Due
 * to it's naturally close integration with OpenComposite, this API is unversioned.
 */
class IBackend {
public:
	typedef int openvr_enum_t;

	DECLARE_BACKEND_FUNCS(virtual, = 0)

	// Virtual Destructor
	virtual ~IBackend();
};

class BackendManager {
public:
	static void Create(IBackend*);
	static BackendManager& Instance();
	static BackendManager* InstancePtr();
	static void Reset();

	// main methods

	DECLARE_BACKEND_FUNCS(, )

	// Legacy, replaced by GetDevice(index)->GetPose(...)
	void GetSinglePose(
	    vr::ETrackingUniverseOrigin origin,
	    vr::TrackedDeviceIndex_t index,
	    vr::TrackedDevicePose_t* pose,
	    ETrackingStateType trackingState);

	static vr::TrackedDevicePose_t InvalidPose();

	// Returns global, absolute high-resolution time in seconds.
	float GetTimeInSeconds();

	// Get the current backend instance
	inline IBackend* GetBackendInstance() { return backend.get(); }

private:
	BackendManager();

	// Required for private dtor
	friend std::default_delete<BackendManager>;
	~BackendManager();

	static std::unique_ptr<BackendManager> instance;

	// For now, just use a single backend
	// and add multiple in later
	std::unique_ptr<IBackend> backend;
};
