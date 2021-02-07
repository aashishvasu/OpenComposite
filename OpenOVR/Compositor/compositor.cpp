#include "stdafx.h"

#include "compositor.h"

Compositor::~Compositor()
{
	if (chain) {
		xrDestroySwapchain(chain);
		chain = XR_NULL_HANDLE;
	}
}
