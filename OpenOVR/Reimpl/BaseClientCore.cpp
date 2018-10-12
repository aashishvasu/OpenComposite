#include "stdafx.h"
#define BASE_IMPL
#include "BaseClientCore.h"
#include "steamvr_abi.h"

using namespace std;
using namespace vr;

EVRInitError BaseClientCore::Init(vr::EVRApplicationType eApplicationType, const char * pStartupInfo) {
	EVRInitError err;
	VR_InitInternal2(&err, eApplicationType, pStartupInfo);
	return err;
}

void BaseClientCore::Cleanup() {
	// Note that this object is not affected by the shutdown, as it is handled seperately
	//  from all the other interface objects and is only destroyed when the DLL is unloaded.
	VR_ShutdownInternal();
}

EVRInitError BaseClientCore::IsInterfaceVersionValid(const char * pchInterfaceVersion) {
	return VR_IsInterfaceVersionValid(pchInterfaceVersion) ? VRInitError_None : VRInitError_Init_InvalidInterface;
}

void * BaseClientCore::GetGenericInterface(const char * pchNameAndVersion, EVRInitError * peError) {
	return VR_GetGenericInterface(pchNameAndVersion, peError);
}

bool BaseClientCore::BIsHmdPresent() {
	return VR_IsHmdPresent();
}

const char * BaseClientCore::GetEnglishStringForHmdError(vr::EVRInitError eError) {
	return VR_GetVRInitErrorAsEnglishDescription(eError);
}

const char * BaseClientCore::GetIDForVRInitError(vr::EVRInitError eError) {
	return VR_GetVRInitErrorAsSymbol(eError);
}
