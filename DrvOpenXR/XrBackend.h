//
// Created by ZNix on 25/10/2020.
//

#pragma once

#include "XrDriverPrivate.h"

#include "XrHMD.h"

#include <memory>

class XrBackend : public IBackend {
public:
	DECLARE_BACKEND_FUNCS(virtual, override);

	virtual ~XrBackend() override;

private:
	std::unique_ptr<XrHMD> hmd = std::make_unique<XrHMD>();

	void CheckOrInitCompositors(const vr::Texture_t* tex);
	std::unique_ptr<Compositor> compositors[XruEyeCount];

	// Whether we've restarted the session to use the application's rendering API yet
	bool usingApplicationGraphicsAPI = false;

	// The views for the two main eye layers
	XrCompositionLayerProjectionView projectionViews[XruEyeCount];

	// Have we started rendering a frame yet? If not, calling xrEndFrame would result in an error
	bool renderingFrame = false;
};
