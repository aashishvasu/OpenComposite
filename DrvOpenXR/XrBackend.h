//
// Created by ZNix on 25/10/2020.
//

#pragma once

#include "XrDriverPrivate.h"

#include "XrController.h"
#include "XrHMD.h"

#include <memory>

class XrBackend : public IBackend {
public:
	DECLARE_BACKEND_FUNCS(virtual, override);

	XrBackend(bool useVulkanTmpGfx, bool useD3D11TmpGfx);
	~XrBackend() override;

	/**
	 * The current state of the OpenXR session, or XR_SESSION_STATE_UNKNOWN if there is no session.
	 */
	XrSessionState sessionState = XR_SESSION_STATE_UNKNOWN;

	XrSessionState GetSessionState();

	/**
	 * Whether the session is active or not. This cannot be determined from just the session state, since
	 * we're allowed to send frames after calling xrBeginSession but before the event comes through. Same
	 * with ending the session - we're allowed to submit frames after we receive the stopping event, until
	 * we call xrEndSession.
	 */
	bool sessionActive = false;

	/**
	 * To be called after xrCreateSession. Should only be used by DrvOpenXR.
	 */
	void OnSessionCreated();

	void PrepareForSessionShutdown();

	const void* GetCurrentGraphicsBinding();

	/**
	 * Restarts the session to allow for inputs to be attached to the session, if necessary.
	 * To be called from BaseInput whenever it's attempting to attach the game actions.
	 * Restarting the session should only be necessary if we've already created the infoSet.
	 */
	static void MaybeRestartForInputs();

#ifdef SUPPORT_VK
	static void VkGetPhysicalDevice(VkInstance instance, VkPhysicalDevice* out);
#endif

private:
	std::unique_ptr<XrHMD> hmd = std::make_unique<XrHMD>();
	std::unique_ptr<XrController> hand_left;
	std::unique_ptr<XrController> hand_right;

	void CheckOrInitCompositors(const vr::Texture_t* tex);
	std::unique_ptr<Compositor> compositors[XruEyeCount];

	/**
	 * Updates the current interaction profile in use according to the runtime.
	 * This will set the XrHMD's interaction profile, as well as create the XrControllers
	 * and set their interaction profiles accordingly.
	 * This allows for games to retrieve correct per-controller OpenVR properties that they request.
	 * Called from PumpEvents on an INTERACTION_PROFILE_CHANGED event.
	 */
	void UpdateInteractionProfile();

	/**
	 * Attempts to force the runtime to expose an interaction profile
	 * (i.e., send an INTERACTION_PROFILE_CHANGED event).
	 * If this is called before the game's actions have been attached to the session by BaseInput
	 * (which is the case for essentially all legacy input games), the session will have to be restarted
	 * to attach the actual inputs. BaseInput handles this when attaching its inputs.
	 */
	void QueryForInteractionProfile();
	void CreateInfoSet();
	void BindInfoSet();

	// Whether we've restarted the session to use the application's rendering API yet
	bool usingApplicationGraphicsAPI = false;

	// The views for the two main eye layers
	XrCompositionLayerProjectionView projectionViews[XruEyeCount];

	// Have we started rendering a frame yet? If not, calling xrEndFrame would result in an error
	bool renderingFrame = false;

	// Were we supposed to start rendering a frame, but couldn't since we were on the
	// early (pre switch to application graphics instance) OpenXR session?
	bool deferredRenderingStart = false;

	// Keep track of if eye textures have been submitted and if we need to create a projection layer for them
	bool submittedEyeTextures = false;

	// If the app is using PostPresentHandoff then we need to delay when we submit frame data through xrEndFrame
	// until after all frame and layer data has been submitted and PostPresentHandoff is called. Otherwise we
	// might miss overlay elements for GUI or HUDs
	bool postPresentStatus = false;

	// Number of frames rendered for use in frame timing data
	uint32_t nFrameIndex = 0;

	double frameSubmitTimeUs = 0.0;

	// Action set and action used for querying for the interaction profile
	inline static XrActionSet infoSet = XR_NULL_HANDLE;
	XrAction infoAction = XR_NULL_HANDLE;
	XrPath subactionPaths[2] = { XR_NULL_PATH, XR_NULL_PATH };

	// Abstract class for holding a graphics binding.
	class BindingBase {
	public:
		virtual const void* asVoid() = 0;
		virtual ~BindingBase() = default;
	};

	// A wrapper around a graphics binding type, subclassing BindingBase
	// It is templated to allow storing any of the possible graphics bindings and getting a void* to it,
	// as necessary for xrCreateSession (in DrvOpenXR::SetupSession)
	template <typename T>
	class BindingWrapper : public BindingBase {
		const T data;

	public:
		BindingWrapper(T data)
		    : data(data) {}
		const void* asVoid() override { return &data; }
	};

	// The current graphics binding, used for restarting the session
	inline static std::unique_ptr<BindingBase> graphicsBinding = nullptr;
	static std::unique_ptr<class TemporaryGraphics> temporaryGraphics;
};
