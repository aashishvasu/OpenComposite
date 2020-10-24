#include "stdafx.h"

#include "compositor.h"

Compositor::~Compositor()
{
	if (chain) {
		xrDestroySwapchain(chain);
		chain = nullptr;
	}
}
