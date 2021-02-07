#pragma once
#include "../BaseCommon.h" // TODO don't import from OCOVR, and remove the "../"
#include "OpenVR/interfaces/IVRCompositor_022.h"

#include "../Compositor/compositor_backend.h" // TODO don't import from OCOVR, and remove the "../"

#include <vector>

typedef unsigned int GLuint;

struct OOVR_Compositor_FrameTiming {
	uint32_t m_nSize; // Set to sizeof( Compositor_FrameTiming )
	uint32_t m_nFrameIndex;
	uint32_t m_nNumFramePresents; // number of times this frame was presented
	uint32_t m_nNumMisPresented; // number of times this frame was presented on a vsync other than it was originally predicted to
	uint32_t m_nNumDroppedFrames; // number of additional times previous frame was scanned out
	uint32_t m_nReprojectionFlags;

	/** Absolute time reference for comparing frames.  This aligns with the vsync that running start is relative to. */
	double m_flSystemTimeInSeconds;

	/** These times may include work from other processes due to OS scheduling.
	* The fewer packets of work these are broken up into, the less likely this will happen.
	* GPU work can be broken up by calling Flush.  This can sometimes be useful to get the GPU started
	* processing that work earlier in the frame. */
	float m_flPreSubmitGpuMs; // time spent rendering the scene (gpu work submitted between WaitGetPoses and second Submit)
	float m_flPostSubmitGpuMs; // additional time spent rendering by application (e.g. companion window)
	float m_flTotalRenderGpuMs; // time between work submitted immediately after present (ideally vsync) until the end of compositor submitted work
	float m_flCompositorRenderGpuMs; // time spend performing distortion correction, rendering chaperone, overlays, etc.
	float m_flCompositorRenderCpuMs; // time spent on cpu submitting the above work for this frame
	float m_flCompositorIdleCpuMs; // time spent waiting for running start (application could have used this much more time)

	/** Miscellaneous measured intervals. */
	float m_flClientFrameIntervalMs; // time between calls to WaitGetPoses
	float m_flPresentCallCpuMs; // time blocked on call to present (usually 0.0, but can go long)
	float m_flWaitForPresentCpuMs; // time spent spin-waiting for frame index to change (not near-zero indicates wait object failure)
	float m_flSubmitFrameMs; // time spent in IVRCompositor::Submit (not near-zero indicates driver issue)

	/** The following are all relative to this frame's SystemTimeInSeconds */
	float m_flWaitGetPosesCalledMs;
	float m_flNewPosesReadyMs;
	float m_flNewFrameReadyMs; // second call to IVRCompositor::Submit
	float m_flCompositorUpdateStartMs;
	float m_flCompositorUpdateEndMs;
	float m_flCompositorRenderStartMs;

	vr::TrackedDevicePose_t m_HmdPose; // pose used by app to render this frame
};

struct OOVR_Compositor_CumulativeStats {
	uint32_t m_nPid; // Process id associated with these stats (may no longer be running).
	uint32_t m_nNumFramePresents; // total number of times we called present (includes reprojected frames)
	uint32_t m_nNumDroppedFrames; // total number of times an old frame was re-scanned out (without reprojection)
	uint32_t m_nNumReprojectedFrames; // total number of times a frame was scanned out a second time (with reprojection)

	/** Values recorded at startup before application has fully faded in the first time. */
	uint32_t m_nNumFramePresentsOnStartup;
	uint32_t m_nNumDroppedFramesOnStartup;
	uint32_t m_nNumReprojectedFramesOnStartup;

	/** Applications may explicitly fade to the compositor.  This is usually to handle level transitions, and loading often causes
	* system wide hitches.  The following stats are collected during this period.  Does not include values recorded during startup. */
	uint32_t m_nNumLoading;
	uint32_t m_nNumFramePresentsLoading;
	uint32_t m_nNumDroppedFramesLoading;
	uint32_t m_nNumReprojectedFramesLoading;

	/** If we don't get a new frame from the app in less than 2.5 frames, then we assume the app has hung and start
	* fading back to the compositor.  The following stats are a result of this, and are a subset of those recorded above.
	* Does not include values recorded during start up or loading. */
	uint32_t m_nNumTimedOut;
	uint32_t m_nNumFramePresentsTimedOut;
	uint32_t m_nNumDroppedFramesTimedOut;
	uint32_t m_nNumReprojectedFramesTimedOut;
};

enum OOVR_EVRCompositorTimingMode {
	VRCompositorTimingMode_Implicit = 0,
	VRCompositorTimingMode_Explicit_RuntimePerformsPostPresentHandoff = 1,
	VRCompositorTimingMode_Explicit_ApplicationPerformsPostPresentHandoff = 2,
};

struct OOVR_Compositor_StageRenderSettings {
	/** Primary color is applied as a tint to (i.e. multiplied with) the model's texture */
	vr::HmdColor_t m_PrimaryColor = { 1, 1, 1, 1 };
	vr::HmdColor_t m_SecondaryColor = { 1, 1, 1, 1 };

	/** Vignette radius is in meters and is used to fade to the specified secondary solid color over
	* that 3D distance from the origin of the playspace. */
	float m_flVignetteInnerRadius = 0.0f;
	float m_flVignetteOuterRadius = 0.0f;

	/** Fades to the secondary color based on view incidence.  This variable controls the linearity
	* of the effect.  It is mutually exclusive with vignette.  Additionally, it treats the mesh as faceted. */
	float m_flFresnelStrength = 0.0f;

	/** Controls backface culling. */
	bool m_bBackfaceCulling = false;

	/** Converts the render model's texture to luma and applies to rgb equally.  This is useful to
	* combat compression artifacts that can occur on desaturated source material. */
	bool m_bGreyscale = false;

	/** Renders mesh as a wireframe. */
	bool m_bWireframe = false;
};

class BaseCompositor {
private:
	bool leftEyeSubmitted = false, rightEyeSubmitted = false;

	vr::HmdColor_t fadeColour = { 0, 0, 0, 0 };
	float fadeTime = 0;

	bool isInSkybox = false;

	// True if the current frame has only nullptr textures supplied, and should be ignored
	bool isNullRender = false;

public:
	typedef int ovr_enum_t;

	BaseCompositor();
	~BaseCompositor();

	// Used in CVRSystem
	void GetSinglePoseRendering(vr::ETrackingUniverseOrigin origin, vr::TrackedDeviceIndex_t index, vr::TrackedDevicePose_t* pose);

	static MfMatrix4f GetHandTransform();

	/** Creates API specific Compositor */
	static Compositor* CreateCompositorAPI(const vr::Texture_t* texture);

#if defined(SUPPORT_DX) && defined(SUPPORT_DX11) && !defined(OC_XR_PORT)
	// TODO clean this up, and make the keyboard work with OpenGL and Vulkan too
	static DX11Compositor* dxcomp;
#endif

public:
	// OpenVR interface methods from here on:

	/** Sets tracking space returned by WaitGetPoses */
	virtual void SetTrackingSpace(vr::ETrackingUniverseOrigin eOrigin);

	/** Gets current tracking space returned by WaitGetPoses */
	virtual vr::ETrackingUniverseOrigin GetTrackingSpace();

	/** Scene applications should call this function to get poses to render with (and optionally poses predicted an additional frame out to use for gameplay).
	* This function will block until "running start" milliseconds before the start of the frame, and should be called at the last moment before needing to
	* start rendering.
	*
	* Return codes:
	*	- IsNotSceneApplication (make sure to call VR_Init with VRApplicaiton_Scene)
	*	- DoNotHaveFocus (some other app has taken focus - this will throttle the call to 10hz to reduce the impact on that app)
	*/
	virtual ovr_enum_t WaitGetPoses(vr::TrackedDevicePose_t* pRenderPoseArray, uint32_t unRenderPoseArrayCount,
	    vr::TrackedDevicePose_t* pGamePoseArray, uint32_t unGamePoseArrayCount);

	/** Get the last set of poses returned by WaitGetPoses. */
	virtual ovr_enum_t GetLastPoses(vr::TrackedDevicePose_t* pRenderPoseArray, uint32_t unRenderPoseArrayCount,
	    vr::TrackedDevicePose_t* pGamePoseArray, uint32_t unGamePoseArrayCount);

	/** Interface for accessing last set of poses returned by WaitGetPoses one at a time.
	* Returns VRCompositorError_IndexOutOfRange if unDeviceIndex not less than k_unMaxTrackedDeviceCount otherwise VRCompositorError_None.
	* It is okay to pass NULL for either pose if you only want one of the values. */
	virtual ovr_enum_t GetLastPoseForTrackedDeviceIndex(vr::TrackedDeviceIndex_t unDeviceIndex, vr::TrackedDevicePose_t* pOutputPose, vr::TrackedDevicePose_t* pOutputGamePose);

	/** Updated scene texture to display. If pBounds is NULL the entire texture will be used.  If called from an OpenGL app, consider adding a glFlush after
	* Submitting both frames to signal the driver to start processing, otherwise it may wait until the command buffer fills up, causing the app to miss frames.
	*
	* OpenGL dirty state:
	*	glBindTexture
	*
	* Return codes:
	*	- IsNotSceneApplication (make sure to call VR_Init with VRApplicaiton_Scene)
	*	- DoNotHaveFocus (some other app has taken focus)
	*	- TextureIsOnWrongDevice (application did not use proper AdapterIndex - see IVRSystem.GetDXGIOutputInfo)
	*	- SharedTexturesNotSupported (application needs to call CreateDXGIFactory1 or later before creating DX device)
	*	- TextureUsesUnsupportedFormat (scene textures must be compatible with DXGI sharing rules - e.g. uncompressed, no mips, etc.)
	*	- InvalidTexture (usually means bad arguments passed in)
	*	- AlreadySubmitted (app has submitted two left textures or two right textures in a single frame - i.e. before calling WaitGetPoses again)
	*/
	virtual ovr_enum_t Submit(vr::EVREye eEye, const vr::Texture_t* pTexture, const vr::VRTextureBounds_t* pBounds, vr::EVRSubmitFlags nSubmitFlags = vr::Submit_Default);

	/** Clears the frame that was sent with the last call to Submit. This will cause the
	* compositor to show the grid until Submit is called again. */
	virtual void ClearLastSubmittedFrame();

	/** Call immediately after presenting your app's window (i.e. companion window) to unblock the compositor.
	* This is an optional call, which only needs to be used if you can't instead call WaitGetPoses immediately after Present.
	* For example, if your engine's render and game loop are not on separate threads, or blocking the render thread until 3ms before the next vsync would
	* introduce a deadlock of some sort.  This function tells the compositor that you have finished all rendering after having Submitted buffers for both
	* eyes, and it is free to start its rendering work.  This should only be called from the same thread you are rendering on. */
	virtual void PostPresentHandoff();

	/** Returns true if timing data is filled it.  Sets oldest timing info if nFramesAgo is larger than the stored history.
	* Be sure to set timing.size = sizeof(Compositor_FrameTiming) on struct passed in before calling this function. */
	virtual bool GetFrameTiming(OOVR_Compositor_FrameTiming* pTiming, uint32_t unFramesAgo);

	/** Interface for copying a range of timing data.  Frames are returned in ascending order (oldest to newest) with the last being the most recent frame.
	* Only the first entry's m_nSize needs to be set, as the rest will be inferred from that.  Returns total number of entries filled out. */
	virtual uint32_t GetFrameTimings(OOVR_Compositor_FrameTiming* pTiming, uint32_t nFrames);

	// The Compositor_FrameTiming type was moved to vrtypes.h, hence the duplicate methods
	virtual bool GetFrameTiming(vr::Compositor_FrameTiming* pTiming, uint32_t unFramesAgo);
	virtual uint32_t GetFrameTimings(vr::Compositor_FrameTiming* pTiming, uint32_t nFrames);

	/** Returns the time in seconds left in the current (as identified by FrameTiming's frameIndex) frame.
	* Due to "running start", this value may roll over to the next frame before ever reaching 0.0. */
	virtual float GetFrameTimeRemaining();

	/** Fills out stats accumulated for the last connected application.  Pass in sizeof( Compositor_CumulativeStats ) as second parameter. */
	virtual void GetCumulativeStats(OOVR_Compositor_CumulativeStats* pStats, uint32_t nStatsSizeInBytes);

	/** Fades the view on the HMD to the specified color. The fade will take fSeconds, and the color values are between
	* 0.0 and 1.0. This color is faded on top of the scene based on the alpha parameter. Removing the fade color instantly
	* would be FadeToColor( 0.0, 0.0, 0.0, 0.0, 0.0 ).  Values are in un-premultiplied alpha space. */
	virtual void FadeToColor(float fSeconds, float fRed, float fGreen, float fBlue, float fAlpha, bool bBackground = false);

	/** Get current fade color value. */
	virtual vr::HmdColor_t GetCurrentFadeColor(bool bBackground = false);

	/** Fading the Grid in or out in fSeconds */
	virtual void FadeGrid(float fSeconds, bool bFadeIn);

	/** Get current alpha value of grid. */
	virtual float GetCurrentGridAlpha();

	/** Override the skybox used in the compositor (e.g. for during level loads when the app can't feed scene images fast enough)
	* Order is Front, Back, Left, Right, Top, Bottom.  If only a single texture is passed, it is assumed in lat-long format.
	* If two are passed, it is assumed a lat-long stereo pair. */
	virtual ovr_enum_t SetSkyboxOverride(const vr::Texture_t* pTextures, uint32_t unTextureCount);

	/** Resets compositor skybox back to defaults. */
	virtual void ClearSkyboxOverride();

	/** Brings the compositor window to the front. This is useful for covering any other window that may be on the HMD
	* and is obscuring the compositor window. */
	virtual void CompositorBringToFront();

	/** Pushes the compositor window to the back. This is useful for allowing other applications to draw directly to the HMD. */
	virtual void CompositorGoToBack();

	/** Tells the compositor process to clean up and exit. You do not need to call this function at shutdown. Under normal
	* circumstances the compositor will manage its own life cycle based on what applications are running. */
	virtual void CompositorQuit();

	/** Return whether the compositor is fullscreen */
	virtual bool IsFullscreen();

	/** Returns the process ID of the process that is currently rendering the scene */
	virtual uint32_t GetCurrentSceneFocusProcess();

	/** Returns the process ID of the process that rendered the last frame (or 0 if the compositor itself rendered the frame.)
	* Returns 0 when fading out from an app and the app's process Id when fading into an app. */
	virtual uint32_t GetLastFrameRenderer();

	/** Returns true if the current process has the scene focus */
	virtual bool CanRenderScene();

	/** Creates a window on the primary monitor to display what is being shown in the headset. */
	virtual void ShowMirrorWindow();

	/** Closes the mirror window. */
	virtual void HideMirrorWindow();

	/** Returns true if the mirror window is shown. */
	virtual bool IsMirrorWindowVisible();

	/** Writes all images that the compositor knows about (including overlays) to a 'screenshots' folder in the SteamVR runtime root. */
	virtual void CompositorDumpImages();

	/** Let an app know it should be rendering with low resources. */
	virtual bool ShouldAppRenderWithLowResources();

	/** Override interleaved reprojection logic to force on. */
	virtual void ForceInterleavedReprojectionOn(bool bOverride);

	/** Force reconnecting to the compositor process. */
	virtual void ForceReconnectProcess();

	/** Temporarily suspends rendering (useful for finer control over scene transitions). */
	virtual void SuspendRendering(bool bSuspend);

	/** Opens a shared D3D11 texture with the undistorted composited image for each eye.  Use ReleaseMirrorTextureD3D11 when finished
	* instead of calling Release on the resource itself. */
	virtual ovr_enum_t GetMirrorTextureD3D11(vr::EVREye eEye, void* pD3D11DeviceOrResource, void** ppD3D11ShaderResourceView);
	virtual void ReleaseMirrorTextureD3D11(void* pD3D11ShaderResourceView);

	/** Access to mirror textures from OpenGL. */
	virtual ovr_enum_t GetMirrorTextureGL(vr::EVREye eEye, vr::glUInt_t* pglTextureId, vr::glSharedTextureHandle_t* pglSharedTextureHandle);
	virtual bool ReleaseSharedGLTexture(vr::glUInt_t glTextureId, vr::glSharedTextureHandle_t glSharedTextureHandle);
	virtual void LockGLSharedTextureForAccess(vr::glSharedTextureHandle_t glSharedTextureHandle);
	virtual void UnlockGLSharedTextureForAccess(vr::glSharedTextureHandle_t glSharedTextureHandle);

	/** [Vulkan Only]
	* return 0. Otherwise it returns the length of the number of bytes necessary to hold this string including the trailing
	* null.  The string will be a space separated list of-required instance extensions to enable in VkCreateInstance */
	virtual uint32_t GetVulkanInstanceExtensionsRequired(char* pchValue, uint32_t unBufferSize);

	/** [Vulkan only]
	* return 0. Otherwise it returns the length of the number of bytes necessary to hold this string including the trailing
	* null.  The string will be a space separated list of required device extensions to enable in VkCreateDevice */
	virtual uint32_t GetVulkanDeviceExtensionsRequired(VkPhysicalDevice_T* pPhysicalDevice, char* pchValue, uint32_t unBufferSize);

	/** [ Vulkan/D3D12 Only ]
	* There are two purposes for SetExplicitTimingMode:
	*	1. To get a more accurate GPU timestamp for when the frame begins in Vulkan/D3D12 applications.
	*	2. (Optional) To avoid having WaitGetPoses access the Vulkan queue so that the queue can be accessed from
	*	another thread while WaitGetPoses is executing.
	*
	* More accurate GPU timestamp for the start of the frame is achieved by the application calling
	* SubmitExplicitTimingData immediately before its first submission to the Vulkan/D3D12 queue.
	* This is more accurate because normally this GPU timestamp is recorded during WaitGetPoses.  In D3D11,
	* WaitGetPoses queues a GPU timestamp write, but it does not actually get submitted to the GPU until the
	* application flushes.  By using SubmitExplicitTimingData, the timestamp is recorded at the same place for
	* Vulkan/D3D12 as it is for D3D11, resulting in a more accurate GPU time measurement for the frame.
	*
	* Avoiding WaitGetPoses accessing the Vulkan queue can be achieved using SetExplicitTimingMode as well.  If this is desired,
	* the application should set the timing mode to Explicit_ApplicationPerformsPostPresentHandoff and *MUST* call PostPresentHandoff
	* itself. If these conditions are met, then WaitGetPoses is guaranteed not to access the queue.  Note that PostPresentHandoff
	* and SubmitExplicitTimingData will access the queue, so only WaitGetPoses becomes safe for accessing the queue from another
	* thread. */
	virtual void SetExplicitTimingMode(ovr_enum_t eTimingMode);

	/** [ Vulkan/D3D12 Only ]
	* Submit explicit timing data.  When SetExplicitTimingMode is true, this must be called immediately before
	* the application's first vkQueueSubmit (Vulkan) or ID3D12CommandQueue::ExecuteCommandLists (D3D12) of each frame.
	* This function will insert a GPU timestamp write just before the application starts its rendering.  This function
	* will perform a vkQueueSubmit on Vulkan so must not be done simultaneously with VkQueue operations on another thread.
	* Returns VRCompositorError_RequestFailed if SetExplicitTimingMode is not enabled. */
	virtual ovr_enum_t SubmitExplicitTimingData();

	/** Indicates whether or not motion smoothing is enabled by the user settings.
	* If you want to know if motion smoothing actually triggered due to a late frame, check Compositor_FrameTiming
	* m_nReprojectionFlags & VRCompositor_ReprojectionMotion instead. */
	virtual bool IsMotionSmoothingEnabled();

	/** Indicates whether or not motion smoothing is supported by the current hardware. */
	virtual bool IsMotionSmoothingSupported();

	/** Indicates whether or not the current scene focus app is currently loading.  This is inferred from its use of FadeGrid to
	* explicitly fade to the compositor to cover up the fact that it cannot render at a sustained full framerate during this time. */
	virtual bool IsCurrentSceneFocusAppLoading();

	/** Override the stage model used in the compositor to replace the grid.  RenderModelPath is a full path the an OBJ file to load.
     * This file will be loaded asynchronously from disk and uploaded to the gpu by the runtime.  Once ready for rendering, the
     * VREvent StageOverrideReady will be sent.  Use FadeToGrid to reveal.  Call ClearStageOverride to free the associated resources when finished. */
	virtual ovr_enum_t SetStageOverride_Async(const char* pchRenderModelPath, const vr::HmdMatrix34_t* pTransform = 0,
	    const OOVR_Compositor_StageRenderSettings* pRenderSettings = 0, uint32_t nSizeOfRenderSettings = 0);

	/** Resets the stage to its default user specified setting. */
	virtual void ClearStageOverride();

	/** Returns true if pBenchmarkResults is filled it.  Sets pBenchmarkResults with the result of the compositor benchmark.
	* nSizeOfBenchmarkResults should be set to sizeof(Compositor_BenchmarkResults) */
	virtual bool GetCompositorBenchmarkResults(vr::Compositor_BenchmarkResults* pBenchmarkResults, uint32_t nSizeOfBenchmarkResults);

	/** Returns the frame id associated with the poses last returned by WaitGetPoses.  Deltas between IDs correspond to
	 * number of headset vsync intervals. */
	virtual ovr_enum_t GetLastPosePredictionIDs(uint32_t* pRenderPosePredictionID, uint32_t* pGamePosePredictionID);

	/** Get the most up-to-date predicted (or recorded - up to 100ms old) set of poses for a given frame id. */
	virtual ovr_enum_t GetPosesForFrame(uint32_t unPosePredictionID, vr::TrackedDevicePose_t* pPoseArray, uint32_t unPoseArrayCount);
};
