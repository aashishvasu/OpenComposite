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

	XrBackend();
	~XrBackend() override;

	/**
	 * The current state of the OpenXR session, or XR_SESSION_STATE_UNKNOWN if there is no session.
	 */
	XrSessionState sessionState = XR_SESSION_STATE_UNKNOWN;

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

private:
	std::unique_ptr<XrHMD> hmd = std::make_unique<XrHMD>();
	std::unique_ptr<XrController> hand_left = std::make_unique<XrController>(XrController::XCT_LEFT);
	std::unique_ptr<XrController> hand_right = std::make_unique<XrController>(XrController::XCT_RIGHT);

	void CheckOrInitCompositors(const vr::Texture_t* tex);
	std::unique_ptr<Compositor> compositors[XruEyeCount];

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
};
