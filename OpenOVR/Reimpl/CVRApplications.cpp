#include "stdafx.h"
#define GENFILE
#include "BaseCommon.h"

// ~0 is all bits true, so it can be used on any apptype except for the bootstrapper
BASE_FLAG([APPTYPE] = ~(1ull << VRApplication_Bootstrapper))

GEN_INTERFACE("Applications", "004")
GEN_INTERFACE("Applications", "005")
GEN_INTERFACE("Applications", "006")
GEN_INTERFACE("Applications", "007")
