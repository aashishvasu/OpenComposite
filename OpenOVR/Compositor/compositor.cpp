#include "stdafx.h"

#include "compositor.h"

Compositor::~Compositor()
{
	if (chain) {
		OOVR_FAILED_XR_ABORT(xrDestroySwapchain(chain));
		chain = XR_NULL_HANDLE;
	}
}
