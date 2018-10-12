#include "stdafx.h"
#define GENFILE
#include "BaseCommon.h"

GEN_INTERFACE("ClientCore", "003", CUSTOM)
GEN_INTERFACE("ClientCore", "002", CUSTOM)

#include "GVRClientCore.gen.h"

vr::EVRInitError CVRClientCore_002::Init(vr::EVRApplicationType eApplicationType) {
	// This is how the DLL does it with the old InitInternal
	return base->Init(eApplicationType, NULL);
}
