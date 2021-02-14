//
// Created by ZNix on 25/10/2020.
//

#pragma once

// FIXME Make it so that this header can be used in the Vulkan compositor - unfortunate and we should probably fix that at some point
#include "pub/DrvOpenXR.h"

#include "../OpenOVR/Misc/xrutil.h"
#include "../OpenOVR/logging.h"

#ifndef _WIN32
#include "../OpenOVR/linux_funcs.h"
#endif

#define STUBBED() \
	OOVR_ABORTF("DrvOpenXR: Hit stubbed file at %s:%d func %s", __FILE__, __LINE__, __func__)

namespace DrvOpenXR {
void SetupSession(const void* graphicsBinding);
void ShutdownSession();
void FullShutdown();
} // namespace DrvOpenXR
