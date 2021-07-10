#pragma once
#include "../interfaces/openvr.h"
#include "../interfaces/vrtypes.h"
#include "../interfaces/vrannotation.h"

// Unfortunately, Valve did not publish the header for version 017 of this interface, and then
//  used it in The Lab. Since that's written in Unity, I was however able to find the differences.
// It seems to just be 018, but without a couple of grid fade functions

namespace vr
{
namespace IVRCompositor_017
{

#if defined(__linux__) || defined(__APPLE__) 
	// The 32-bit version of gcc has the alignment requirement for uint64 and double set to
	// 4 meaning that even with #pragma pack(8) these types will only be four-byte aligned.
	// The 64-bit version of gcc has the alignment requirement for these types set to
	// 8 meaning that unless we use #pragma pack(4) our structures will get bigger.
	// The 64-bit structure packing has to match the 32-bit structure packing for each platform.
	#pragma pack( push, 4 )
#else
	#pragma pack( push, 8 )
#endif

/** Errors that can occur with the VR compositor */
enum EVRCompositorError
{
	VRCompositorError_None						= 0,
	VRCompositorError_RequestFailed				= 1,
	VRCompositorError_IncompatibleVersion		= 100,
	VRCompositorError_DoNotHaveFocus			= 101,
	VRCompositorError_InvalidTexture			= 102,
	VRCompositorError_IsNotSceneApplication		= 103,
	VRCompositorError_TextureIsOnWrongDevice	= 104,
	VRCompositorError_TextureUsesUnsupportedFormat = 105,
	VRCompositorError_SharedTexturesNotSupported = 106,
	VRCompositorError_IndexOutOfRange			= 107,
	VRCompositorError_AlreadySubmitted			= 108,
};

const uint32_t VRCompositor_ReprojectionReason_Cpu = 0x01;
const uint32_t VRCompositor_ReprojectionReason_Gpu = 0x02;
const uint32_t VRCompositor_ReprojectionAsync      = 0x04;	// This flag indicates the async reprojection mode is active,
															// but does not indicate if reprojection actually happened or not.
															// Use the ReprojectionReason flags above to check if reprojection
															// was actually applied (i.e. scene texture was reused).
															// NumFramePresents > 1 also indicates the scene texture was reused,
															// and also the number of times that it was presented in total.

/** Provides a single frame's timing information to the app */
struct Compositor_FrameTiming
{
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

	TrackedDevicePose_t m_HmdPose; // pose used by app to render this frame
};

/** Cumulative stats for current application.  These are not cleared until a new app connects,
* but they do stop accumulating once the associated app disconnects. */
struct Compositor_CumulativeStats
{
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

#pragma pack( pop )

/** Allows the application to interact with the compositor */
class IVRCompositor
{
public:
	/** Sets tracking space returned by WaitGetPoses */
	virtual void SetTrackingSpace( ETrackingUniverseOrigin eOrigin ) = 0;

	/** Gets current tracking space returned by WaitGetPoses */
	virtual ETrackingUniverseOrigin GetTrackingSpace() = 0;

	/** Scene applications should call this function to get poses to render with (and optionally poses predicted an additional frame out to use for gameplay).
	* This function will block until "running start" milliseconds before the start of the frame, and should be called at the last moment before needing to
	* start rendering.
	*
	* Return codes:
	*	- IsNotSceneApplication (make sure to call VR_Init with VRApplicaiton_Scene)
	*	- DoNotHaveFocus (some other app has taken focus - this will throttle the call to 10hz to reduce the impact on that app)
	*/
	virtual EVRCompositorError WaitGetPoses( VR_ARRAY_COUNT(unRenderPoseArrayCount) TrackedDevicePose_t* pRenderPoseArray, uint32_t unRenderPoseArrayCount,
		VR_ARRAY_COUNT(unGamePoseArrayCount) TrackedDevicePose_t* pGamePoseArray, uint32_t unGamePoseArrayCount ) = 0;

	/** Get the last set of poses returned by WaitGetPoses. */
	virtual EVRCompositorError GetLastPoses( VR_ARRAY_COUNT( unRenderPoseArrayCount ) TrackedDevicePose_t* pRenderPoseArray, uint32_t unRenderPoseArrayCount,
		VR_ARRAY_COUNT( unGamePoseArrayCount ) TrackedDevicePose_t* pGamePoseArray, uint32_t unGamePoseArrayCount ) = 0;

	/** Interface for accessing last set of poses returned by WaitGetPoses one at a time.
	* Returns VRCompositorError_IndexOutOfRange if unDeviceIndex not less than k_unMaxTrackedDeviceCount otherwise VRCompositorError_None.
	* It is okay to pass NULL for either pose if you only want one of the values. */
	virtual EVRCompositorError GetLastPoseForTrackedDeviceIndex( TrackedDeviceIndex_t unDeviceIndex, TrackedDevicePose_t *pOutputPose, TrackedDevicePose_t *pOutputGamePose ) = 0;

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
	virtual EVRCompositorError Submit( EVREye eEye, const Texture_t *pTexture, const VRTextureBounds_t* pBounds = 0, EVRSubmitFlags nSubmitFlags = Submit_Default ) = 0;

	/** Clears the frame that was sent with the last call to Submit. This will cause the 
	* compositor to show the grid until Submit is called again. */
	virtual void ClearLastSubmittedFrame() = 0;

	/** Call immediately after presenting your app's window (i.e. companion window) to unblock the compositor.
	* This is an optional call, which only needs to be used if you can't instead call WaitGetPoses immediately after Present.
	* For example, if your engine's render and game loop are not on separate threads, or blocking the render thread until 3ms before the next vsync would
	* introduce a deadlock of some sort.  This function tells the compositor that you have finished all rendering after having Submitted buffers for both
	* eyes, and it is free to start its rendering work.  This should only be called from the same thread you are rendering on. */
	virtual void PostPresentHandoff() = 0;

	/** Returns true if timing data is filled it.  Sets oldest timing info if nFramesAgo is larger than the stored history.
	* Be sure to set timing.size = sizeof(Compositor_FrameTiming) on struct passed in before calling this function. */
	virtual bool GetFrameTiming( Compositor_FrameTiming *pTiming, uint32_t unFramesAgo = 0 ) = 0;

	/** Interface for copying a range of timing data.  Frames are returned in ascending order (oldest to newest) with the last being the most recent frame.
	* Only the first entry's m_nSize needs to be set, as the rest will be inferred from that.  Returns total number of entries filled out. */
	virtual uint32_t GetFrameTimings( Compositor_FrameTiming *pTiming, uint32_t nFrames ) = 0;

	/** Returns the time in seconds left in the current (as identified by FrameTiming's frameIndex) frame.
	* Due to "running start", this value may roll over to the next frame before ever reaching 0.0. */
	virtual float GetFrameTimeRemaining() = 0;

	/** Fills out stats accumulated for the last connected application.  Pass in sizeof( Compositor_CumulativeStats ) as second parameter. */
	virtual void GetCumulativeStats( Compositor_CumulativeStats *pStats, uint32_t nStatsSizeInBytes ) = 0;

	/** Fades the view on the HMD to the specified color. The fade will take fSeconds, and the color values are between
	* 0.0 and 1.0. This color is faded on top of the scene based on the alpha parameter. Removing the fade color instantly 
	* would be FadeToColor( 0.0, 0.0, 0.0, 0.0, 0.0 ).  Values are in un-premultiplied alpha space. */
	virtual void FadeToColor( float fSeconds, float fRed, float fGreen, float fBlue, float fAlpha, bool bBackground = false ) = 0;

	/** Fading the Grid in or out in fSeconds */
	virtual void FadeGrid( float fSeconds, bool bFadeIn ) = 0;

	/** Override the skybox used in the compositor (e.g. for during level loads when the app can't feed scene images fast enough)
	* Order is Front, Back, Left, Right, Top, Bottom.  If only a single texture is passed, it is assumed in lat-long format.
	* If two are passed, it is assumed a lat-long stereo pair. */
	virtual EVRCompositorError SetSkyboxOverride( VR_ARRAY_COUNT( unTextureCount ) const Texture_t *pTextures, uint32_t unTextureCount ) = 0;

	/** Resets compositor skybox back to defaults. */
	virtual void ClearSkyboxOverride() = 0;

	/** Brings the compositor window to the front. This is useful for covering any other window that may be on the HMD
	* and is obscuring the compositor window. */
	virtual void CompositorBringToFront() = 0;

	/** Pushes the compositor window to the back. This is useful for allowing other applications to draw directly to the HMD. */
	virtual void CompositorGoToBack() = 0;

	/** Tells the compositor process to clean up and exit. You do not need to call this function at shutdown. Under normal 
	* circumstances the compositor will manage its own life cycle based on what applications are running. */
	virtual void CompositorQuit() = 0;
	
	/** Return whether the compositor is fullscreen */
	virtual bool IsFullscreen() = 0;

	/** Returns the process ID of the process that is currently rendering the scene */
	virtual uint32_t GetCurrentSceneFocusProcess() = 0;

	/** Returns the process ID of the process that rendered the last frame (or 0 if the compositor itself rendered the frame.)
	* Returns 0 when fading out from an app and the app's process Id when fading into an app. */
	virtual uint32_t GetLastFrameRenderer() = 0;

	/** Returns true if the current process has the scene focus */
	virtual bool CanRenderScene() = 0;

	/** Creates a window on the primary monitor to display what is being shown in the headset. */
	virtual void ShowMirrorWindow() = 0;

	/** Closes the mirror window. */
	virtual void HideMirrorWindow() = 0;

	/** Returns true if the mirror window is shown. */
	virtual bool IsMirrorWindowVisible() = 0;

	/** Writes all images that the compositor knows about (including overlays) to a 'screenshots' folder in the SteamVR runtime root. */
	virtual void CompositorDumpImages() = 0;

	/** Let an app know it should be rendering with low resources. */
	virtual bool ShouldAppRenderWithLowResources() = 0;

	/** Override interleaved reprojection logic to force on. */
	virtual void ForceInterleavedReprojectionOn( bool bOverride ) = 0;

	/** Force reconnecting to the compositor process. */
	virtual void ForceReconnectProcess() = 0;

	/** Temporarily suspends rendering (useful for finer control over scene transitions). */
	virtual void SuspendRendering( bool bSuspend ) = 0;

	/** Opens a shared D3D11 texture with the undistorted composited image for each eye. */
	virtual EVRCompositorError GetMirrorTextureD3D11( EVREye eEye, void *pD3D11DeviceOrResource, void **ppD3D11ShaderResourceView ) = 0;

	/** Access to mirror textures from OpenGL. */
	virtual EVRCompositorError GetMirrorTextureGL( EVREye eEye, glUInt_t *pglTextureId, glSharedTextureHandle_t *pglSharedTextureHandle ) = 0;
	virtual bool ReleaseSharedGLTexture( glUInt_t glTextureId, glSharedTextureHandle_t glSharedTextureHandle ) = 0;
	virtual void LockGLSharedTextureForAccess( glSharedTextureHandle_t glSharedTextureHandle ) = 0;
	virtual void UnlockGLSharedTextureForAccess( glSharedTextureHandle_t glSharedTextureHandle ) = 0;
};

static const char * const IVRCompositor_Version = "IVRCompositor_017";

} // namespace vr



} // Close custom namespace
