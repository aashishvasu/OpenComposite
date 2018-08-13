#pragma once
#include "OpenVR/interfaces/IVRCompositor_022.h"
#include "BaseCompositor.h"

namespace CVRCompositor_022_scope {

	// FIXME don't use namespaces like this in a header!
	using namespace vr;
	using namespace IVRCompositor_022;

	//!! SET_CLASS_TYPE(CVRCommon)
	class CVRCompositor_022 : IVRCompositor_022::IVRCompositor, public CVRCommon {
		CVR_GEN_IFACE();

	private:
		BaseCompositor base;

	public:
		CVRCompositor_022();
		~CVRCompositor_022();

		// Used in CVRSystem
		ovrTrackingState trackingState;
		ovrSessionStatus sessionStatus;

		/** Sets tracking space returned by WaitGetPoses */
		INTERFACE_FUNC(void, SetTrackingSpace, ETrackingUniverseOrigin eOrigin);

		/** Gets current tracking space returned by WaitGetPoses */
		INTERFACE_FUNC(ETrackingUniverseOrigin, GetTrackingSpace);

		/** Scene applications should call this function to get poses to render with (and optionally poses predicted an additional frame out to use for gameplay).
		* This function will block until "running start" milliseconds before the start of the frame, and should be called at the last moment before needing to
		* start rendering.
		*
		* Return codes:
		*	- IsNotSceneApplication (make sure to call VR_Init with VRApplicaiton_Scene)
		*	- DoNotHaveFocus (some other app has taken focus - this will throttle the call to 10hz to reduce the impact on that app)
		*/
		virtual EVRCompositorError WaitGetPoses(VR_ARRAY_COUNT(unRenderPoseArrayCount) TrackedDevicePose_t* pRenderPoseArray, uint32_t unRenderPoseArrayCount,
			VR_ARRAY_COUNT(unGamePoseArrayCount) TrackedDevicePose_t* pGamePoseArray, uint32_t unGamePoseArrayCount);

		/** Get the last set of poses returned by WaitGetPoses. */
		virtual EVRCompositorError GetLastPoses(VR_ARRAY_COUNT(unRenderPoseArrayCount) TrackedDevicePose_t* pRenderPoseArray, uint32_t unRenderPoseArrayCount,
			VR_ARRAY_COUNT(unGamePoseArrayCount) TrackedDevicePose_t* pGamePoseArray, uint32_t unGamePoseArrayCount);

		/** Interface for accessing last set of poses returned by WaitGetPoses one at a time.
		* Returns VRCompositorError_IndexOutOfRange if unDeviceIndex not less than k_unMaxTrackedDeviceCount otherwise VRCompositorError_None.
		* It is okay to pass NULL for either pose if you only want one of the values. */
		INTERFACE_FUNC(EVRCompositorError, GetLastPoseForTrackedDeviceIndex, TrackedDeviceIndex_t unDeviceIndex, TrackedDevicePose_t *pOutputPose, TrackedDevicePose_t *pOutputGamePose);

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
		INTERFACE_FUNC(EVRCompositorError, Submit, EVREye eEye, const Texture_t *pTexture, const VRTextureBounds_t* pBounds, EVRSubmitFlags nSubmitFlags = Submit_Default);

		/** Clears the frame that was sent with the last call to Submit. This will cause the
		* compositor to show the grid until Submit is called again. */
		INTERFACE_FUNC(void, ClearLastSubmittedFrame);

		/** Call immediately after presenting your app's window (i.e. companion window) to unblock the compositor.
		* This is an optional call, which only needs to be used if you can't instead call WaitGetPoses immediately after Present.
		* For example, if your engine's render and game loop are not on separate threads, or blocking the render thread until 3ms before the next vsync would
		* introduce a deadlock of some sort.  This function tells the compositor that you have finished all rendering after having Submitted buffers for both
		* eyes, and it is free to start its rendering work.  This should only be called from the same thread you are rendering on. */
		INTERFACE_FUNC(void, PostPresentHandoff);

		/** Returns true if timing data is filled it.  Sets oldest timing info if nFramesAgo is larger than the stored history.
		* Be sure to set timing.size = sizeof(Compositor_FrameTiming) on struct passed in before calling this function. */
		INTERFACE_FUNC(bool, GetFrameTiming, Compositor_FrameTiming *pTiming, uint32_t unFramesAgo);

		/** Interface for copying a range of timing data.  Frames are returned in ascending order (oldest to newest) with the last being the most recent frame.
		* Only the first entry's m_nSize needs to be set, as the rest will be inferred from that.  Returns total number of entries filled out. */
		INTERFACE_FUNC(uint32_t, GetFrameTimings, Compositor_FrameTiming *pTiming, uint32_t nFrames);

		/** Returns the time in seconds left in the current (as identified by FrameTiming's frameIndex) frame.
		* Due to "running start", this value may roll over to the next frame before ever reaching 0.0. */
		INTERFACE_FUNC(float, GetFrameTimeRemaining);

		/** Fills out stats accumulated for the last connected application.  Pass in sizeof( Compositor_CumulativeStats ) as second parameter. */
		INTERFACE_FUNC(void, GetCumulativeStats, Compositor_CumulativeStats *pStats, uint32_t nStatsSizeInBytes);

		/** Fades the view on the HMD to the specified color. The fade will take fSeconds, and the color values are between
		* 0.0 and 1.0. This color is faded on top of the scene based on the alpha parameter. Removing the fade color instantly
		* would be FadeToColor( 0.0, 0.0, 0.0, 0.0, 0.0 ).  Values are in un-premultiplied alpha space. */
		INTERFACE_FUNC(void, FadeToColor, float fSeconds, float fRed, float fGreen, float fBlue, float fAlpha, bool bBackground = false);

		/** Get current fade color value. */
		INTERFACE_FUNC(HmdColor_t, GetCurrentFadeColor, bool bBackground = false);

		/** Fading the Grid in or out in fSeconds */
		INTERFACE_FUNC(void, FadeGrid, float fSeconds, bool bFadeIn);

		/** Get current alpha value of grid. */
		INTERFACE_FUNC(float, GetCurrentGridAlpha);

		/** Override the skybox used in the compositor (e.g. for during level loads when the app can't feed scene images fast enough)
		* Order is Front, Back, Left, Right, Top, Bottom.  If only a single texture is passed, it is assumed in lat-long format.
		* If two are passed, it is assumed a lat-long stereo pair. */
		INTERFACE_FUNC(EVRCompositorError, SetSkyboxOverride, VR_ARRAY_COUNT(unTextureCount) const Texture_t *pTextures, uint32_t unTextureCount);

		/** Resets compositor skybox back to defaults. */
		INTERFACE_FUNC(void, ClearSkyboxOverride);

		/** Brings the compositor window to the front. This is useful for covering any other window that may be on the HMD
		* and is obscuring the compositor window. */
		INTERFACE_FUNC(void, CompositorBringToFront);

		/** Pushes the compositor window to the back. This is useful for allowing other applications to draw directly to the HMD. */
		INTERFACE_FUNC(void, CompositorGoToBack);

		/** Tells the compositor process to clean up and exit. You do not need to call this function at shutdown. Under normal
		* circumstances the compositor will manage its own life cycle based on what applications are running. */
		INTERFACE_FUNC(void, CompositorQuit);

		/** Return whether the compositor is fullscreen */
		INTERFACE_FUNC(bool, IsFullscreen);

		/** Returns the process ID of the process that is currently rendering the scene */
		INTERFACE_FUNC(uint32_t, GetCurrentSceneFocusProcess);

		/** Returns the process ID of the process that rendered the last frame (or 0 if the compositor itself rendered the frame.)
		* Returns 0 when fading out from an app and the app's process Id when fading into an app. */
		INTERFACE_FUNC(uint32_t, GetLastFrameRenderer);

		/** Returns true if the current process has the scene focus */
		INTERFACE_FUNC(bool, CanRenderScene);

		/** Creates a window on the primary monitor to display what is being shown in the headset. */
		INTERFACE_FUNC(void, ShowMirrorWindow);

		/** Closes the mirror window. */
		INTERFACE_FUNC(void, HideMirrorWindow);

		/** Returns true if the mirror window is shown. */
		INTERFACE_FUNC(bool, IsMirrorWindowVisible);

		/** Writes all images that the compositor knows about (including overlays) to a 'screenshots' folder in the SteamVR runtime root. */
		INTERFACE_FUNC(void, CompositorDumpImages);

		/** Let an app know it should be rendering with low resources. */
		INTERFACE_FUNC(bool, ShouldAppRenderWithLowResources);

		/** Override interleaved reprojection logic to force on. */
		INTERFACE_FUNC(void, ForceInterleavedReprojectionOn, bool bOverride);

		/** Force reconnecting to the compositor process. */
		INTERFACE_FUNC(void, ForceReconnectProcess);

		/** Temporarily suspends rendering (useful for finer control over scene transitions). */
		INTERFACE_FUNC(void, SuspendRendering, bool bSuspend);

		/** Opens a shared D3D11 texture with the undistorted composited image for each eye.  Use ReleaseMirrorTextureD3D11 when finished
		* instead of calling Release on the resource itself. */
		INTERFACE_FUNC(EVRCompositorError, GetMirrorTextureD3D11, EVREye eEye, void *pD3D11DeviceOrResource, void **ppD3D11ShaderResourceView);
		INTERFACE_FUNC(void, ReleaseMirrorTextureD3D11, void *pD3D11ShaderResourceView);

		/** Access to mirror textures from OpenGL. */
		INTERFACE_FUNC(EVRCompositorError, GetMirrorTextureGL, EVREye eEye, glUInt_t *pglTextureId, glSharedTextureHandle_t *pglSharedTextureHandle);
		INTERFACE_FUNC(bool, ReleaseSharedGLTexture, glUInt_t glTextureId, glSharedTextureHandle_t glSharedTextureHandle);
		INTERFACE_FUNC(void, LockGLSharedTextureForAccess, glSharedTextureHandle_t glSharedTextureHandle);
		INTERFACE_FUNC(void, UnlockGLSharedTextureForAccess, glSharedTextureHandle_t glSharedTextureHandle);

		/** [Vulkan Only]
		* return 0. Otherwise it returns the length of the number of bytes necessary to hold this string including the trailing
		* null.  The string will be a space separated list of-required instance extensions to enable in VkCreateInstance */
		INTERFACE_FUNC(uint32_t, GetVulkanInstanceExtensionsRequired, VR_OUT_STRING() char *pchValue, uint32_t unBufferSize);

		/** [Vulkan only]
		* return 0. Otherwise it returns the length of the number of bytes necessary to hold this string including the trailing
		* null.  The string will be a space separated list of required device extensions to enable in VkCreateDevice */
		INTERFACE_FUNC(uint32_t, GetVulkanDeviceExtensionsRequired, VkPhysicalDevice_T *pPhysicalDevice, VR_OUT_STRING() char *pchValue, uint32_t unBufferSize);

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
		INTERFACE_FUNC(void, SetExplicitTimingMode, IVRCompositor_022::EVRCompositorTimingMode eTimingMode);

		/** [ Vulkan/D3D12 Only ]
		* Submit explicit timing data.  When SetExplicitTimingMode is true, this must be called immediately before
		* the application's first vkQueueSubmit (Vulkan) or ID3D12CommandQueue::ExecuteCommandLists (D3D12) of each frame.
		* This function will insert a GPU timestamp write just before the application starts its rendering.  This function
		* will perform a vkQueueSubmit on Vulkan so must not be done simultaneously with VkQueue operations on another thread.
		* Returns VRCompositorError_RequestFailed if SetExplicitTimingMode is not enabled. */
		virtual IVRCompositor_022::EVRCompositorError SubmitExplicitTimingData();
	};
}

using CVRCompositor_022 = CVRCompositor_022_scope::CVRCompositor_022;