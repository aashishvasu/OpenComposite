#pragma once
#include "BaseCommon.h"

class BaseClientCore {
public:
	static bool CheckAppEnabled();
public:
	/** Initializes the system */
	virtual vr::EVRInitError Init(vr::EVRApplicationType eApplicationType, const char *pStartupInfo);

	/** cleans up everything in vrclient.dll and prepares the DLL to be unloaded */
	virtual void Cleanup();

	/** checks to see if the specified interface/version is supported in this vrclient.dll */
	virtual vr::EVRInitError IsInterfaceVersionValid(const char *pchInterfaceVersion);

	/** Retrieves any interface from vrclient.dll */
	virtual void *GetGenericInterface(const char *pchNameAndVersion, vr::EVRInitError *peError);

	/** Returns true if any driver has an HMD attached. Can be called outside of Init/Cleanup */
	virtual bool BIsHmdPresent();

	/** Returns an English error string from inside vrclient.dll which might be newer than the API DLL */
	virtual const char *GetEnglishStringForHmdError(vr::EVRInitError eError);

	/** Returns an error symbol from inside vrclient.dll which might be newer than the API DLL */
	virtual const char *GetIDForVRInitError(vr::EVRInitError eError);
};
