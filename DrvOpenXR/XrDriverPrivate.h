//
// Created by ZNix on 25/10/2020.
//

#pragma once

#include "DrvOpenXR.h"

#include "../OpenOVR/Misc/xrutil.h"
#include "../OpenOVR/logging.h"

#define STUBBED() \
	OOVR_ABORTF("DrvOpenXR: Hit stubbed file at %s:%d func %s", __FILE__, __LINE__, __func__)

namespace DrvOpenXR {
void SetupSession(void* graphicsBinding);
void ShutdownSession();
}
