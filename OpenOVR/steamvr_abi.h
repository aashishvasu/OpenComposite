#pragma once

#include "stdafx.h"

using namespace vr;

// Please note that, when referring to 'openvr.h', I'm referring to the standard one that has
// little bits of SteamVR embedded into it, not the one in our includes directory that has that cut out.

// Entries arranged in the order they appear in IDA:

/** Returns the interface of the specified version. This method must be called after VR_Init. The
* pointer returned is valid until VR_Shutdown is called.
*/
VR_INTERFACE void *VR_CALLTYPE VR_GetGenericInterface(const char *pchInterfaceVersion, EVRInitError *peError);

/** Returns a token that represents whether the VR interface handles need to be reloaded */
VR_INTERFACE uint32_t VR_CALLTYPE VR_GetInitToken();

// Missing from the openvr.h header
VR_INTERFACE char* VR_GetStringForHmdError(int err);

/** Returns an English string for an EVRInitError. Applications should call VR_GetVRInitErrorAsSymbol instead and
* use that as a key to look up their own localized error message. This function may be called outside of VR_Init()/VR_Shutdown(). */
VR_INTERFACE const char *VR_CALLTYPE VR_GetVRInitErrorAsEnglishDescription(EVRInitError error);

/** Returns the name of the enum value for an EVRInitError. This function may be called outside of VR_Init()/VR_Shutdown(). */
VR_INTERFACE const char *VR_CALLTYPE VR_GetVRInitErrorAsSymbol(EVRInitError error);

// InitInternal calls InitInternal2 with NULL as the last argument.
VR_INTERFACE uint32_t VR_CALLTYPE VR_InitInternal(EVRInitError *peError, EVRApplicationType eApplicationType);
VR_INTERFACE uint32_t VR_CALLTYPE VR_InitInternal2(EVRInitError *peError, EVRApplicationType eApplicationType, const char *pStartupInfo);

/** Returns true if there is an HMD attached. This check is as lightweight as possible and
* can be called outside of VR_Init/VR_Shutdown. It should be used when an application wants
* to know if initializing VR is a possibility but isn't ready to take that step yet.
*/
VR_INTERFACE bool VR_CALLTYPE VR_IsHmdPresent();

/** Returns whether the interface of the specified version exists.
*/
VR_INTERFACE bool VR_CALLTYPE VR_IsInterfaceVersionValid(const char *pchInterfaceVersion);

/** Returns true if the OpenVR runtime is installed. */
VR_INTERFACE bool VR_CALLTYPE VR_IsRuntimeInstalled();

/** Returns where the OpenVR runtime is installed. */
VR_INTERFACE const char *VR_CALLTYPE VR_RuntimePath();

// In openvr.h, but missing any documentation
VR_INTERFACE void VR_CALLTYPE VR_ShutdownInternal();

// The one and only vrclient function:
VR_INTERFACE void* VRClientCoreFactory(const char *pInterfaceName, int *pReturnCode);
