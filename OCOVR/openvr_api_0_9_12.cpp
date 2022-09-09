// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "steamvr_abi.h"

// Only use this on Windows. This is because on Linux, where ELF uses a single pool of symbol names rather 
// than namespacing them by DLL. Thus in the following somewhat obscure case you could have issues when 
// using modern versions of OpenVR:
//
// - The compiler decided not to inline one of the functions from openvr.h (unlikely but possible)
// - The VR-related code is in a shared library loaded with RTLD_LAZY (could certainly happen for a game 
//   engine with a module system)
// - The function is only used after VR initialisation (highly likely - you wouldn't likely call VRCompositor 
//   before startup)
//
// It's also unlikely this will be needed on Linux.

#ifdef _WIN32


namespace OpenVR_v0_9_12 {

static const char* const IVRSystem_Version = "IVRSystem_009";
static const char* const IVRChaperone_Version = "IVRChaperone_003";
static const char* const IVRChaperoneSetup_Version = "IVRChaperoneSetup_004";
static const char* const IVRCompositor_Version = "IVRCompositor_009";
static const char* const IVROverlay_Version = "IVROverlay_007";
static const char* const IVRRenderModels_Version = "IVRRenderModels_002";
static const char* const IVRExtendedDisplay_Version = "IVRExtendedDisplay_001";
static const char* const IVRSettings_Version = "IVRSettings_001";
static const char* const IVRApplications_Version = "IVRApplications_002";
static const char* const IVRTrackedCamera_Version = "IVRTrackedCamera_001";
static const char* const IVRNotifications_Version = "IVRNotifications_002";

inline uint32_t& VRToken()
{
	static uint32_t token;
	return token;
}

inline void outputError(const char* function, const EVRInitError& eError)
{
	if (eError != VRInitError_None) {
		OOVR_LOGF("%s : %s", function, VR_GetVRInitErrorAsEnglishDescription(eError));
	}
}

class COpenVRContext {
public:
	COpenVRContext() { Clear(); }
	void Clear();

	inline void CheckClear()
	{
		if (VRToken() != VR_GetInitToken()) {
			Clear();
			VRToken() = VR_GetInitToken();
		}
	}

	void* VRSystem()
	{
		CheckClear();
		if (m_pVRSystem == nullptr) {
			EVRInitError eError;
			m_pVRSystem = VR_GetGenericInterface(IVRSystem_Version, &eError);
			outputError(IVRSystem_Version, eError);
		}
		return m_pVRSystem;
	}
	void* VRChaperone()
	{
		CheckClear();
		if (m_pVRChaperone == nullptr) {
			EVRInitError eError;
			m_pVRChaperone = VR_GetGenericInterface(IVRChaperone_Version, &eError);
			outputError(IVRChaperone_Version, eError);
		}
		return m_pVRChaperone;
	}

	void* VRChaperoneSetup()
	{
		CheckClear();
		if (m_pVRChaperoneSetup == nullptr) {
			EVRInitError eError;
			m_pVRChaperoneSetup = (void*)VR_GetGenericInterface(IVRChaperoneSetup_Version, &eError);
			outputError(IVRChaperoneSetup_Version, eError);
		}
		return m_pVRChaperoneSetup;
	}

	void* VRCompositor()
	{
		CheckClear();
		if (m_pVRCompositor == nullptr) {
			EVRInitError eError;
			m_pVRCompositor = (void*)VR_GetGenericInterface(IVRCompositor_Version, &eError);
			outputError(IVRCompositor_Version, eError);
		}
		return m_pVRCompositor;
	}

	void* VROverlay()
	{
		CheckClear();
		if (m_pVROverlay == nullptr) {
			EVRInitError eError;
			m_pVROverlay = (void*)VR_GetGenericInterface(IVROverlay_Version, &eError);
			outputError(IVROverlay_Version, eError);
		}
		return m_pVROverlay;
	}

	void* VRRenderModels()
	{
		CheckClear();
		if (m_pVRRenderModels == nullptr) {
			EVRInitError eError;
			m_pVRRenderModels = (void*)VR_GetGenericInterface(IVRRenderModels_Version, &eError);
			outputError(IVRRenderModels_Version, eError);
		}
		return m_pVRRenderModels;
	}

	void* VRExtendedDisplay()
	{
		CheckClear();
		if (m_pVRExtendedDisplay == nullptr) {
			EVRInitError eError;
			m_pVRExtendedDisplay = (void*)VR_GetGenericInterface(IVRExtendedDisplay_Version, &eError);
			outputError(IVRExtendedDisplay_Version, eError);
		}
		return m_pVRExtendedDisplay;
	}

	void* VRSettings()
	{
		CheckClear();
		if (m_pVRSettings == nullptr) {
			EVRInitError eError;
			m_pVRSettings = (void*)VR_GetGenericInterface(IVRSettings_Version, &eError);
			outputError(IVRSettings_Version, eError);
		}
		return m_pVRSettings;
	}

	void* VRApplications()
	{
		CheckClear();
		if (m_pVRApplications == nullptr) {
			EVRInitError eError;
			m_pVRApplications = (void*)VR_GetGenericInterface(IVRApplications_Version, &eError);
			outputError(IVRApplications_Version, eError);
		}
		return m_pVRApplications;
	}

	void* VRTrackedCamera()
	{
		CheckClear();
		if (m_pVRTrackedCamera == nullptr) {
			EVRInitError eError;
			m_pVRTrackedCamera = (void*)VR_GetGenericInterface(IVRTrackedCamera_Version, &eError);
			outputError(IVRTrackedCamera_Version, eError);
		}
		return m_pVRTrackedCamera;
	}

	void* VRNotifications()
	{
		CheckClear();
		if (!m_pVRNotifications) {
			EVRInitError eError;
			m_pVRNotifications = (void*)VR_GetGenericInterface(IVRNotifications_Version, &eError);
			outputError(IVRNotifications_Version, eError);
		}
		return m_pVRNotifications;
	}

private:
	void* m_pVRSystem;
	void* m_pVRChaperone;
	void* m_pVRChaperoneSetup;
	void* m_pVRCompositor;
	void* m_pVROverlay;
	void* m_pVRRenderModels;
	void* m_pVRExtendedDisplay;
	void* m_pVRSettings;
	void* m_pVRApplications;
	void* m_pVRTrackedCamera;
	void* m_pVRNotifications;
};

inline COpenVRContext& OpenVRInternal_ModuleContext()
{
	static void* ctx[sizeof(COpenVRContext) / sizeof(void*)];
	return *(COpenVRContext*)ctx; // bypass zero-init constructor
}

inline void COpenVRContext::Clear()
{
	m_pVRSystem = nullptr;
	m_pVRChaperone = nullptr;
	m_pVRChaperoneSetup = nullptr;
	m_pVRCompositor = nullptr;
	m_pVROverlay = nullptr;
	m_pVRRenderModels = nullptr;
	m_pVRExtendedDisplay = nullptr;
	m_pVRSettings = nullptr;
	m_pVRApplications = nullptr;
	m_pVRTrackedCamera = nullptr;
	m_pVRNotifications = nullptr;
}

} // namespace OpenVR_v0_9_12

// In later versions of OpenVR the following functions are part of openvr.h rather than being implemented in the c++ file.
VR_INTERFACE void* VR_CALLTYPE VRSystem();
VR_INTERFACE void* VR_CALLTYPE VRChaperone();
VR_INTERFACE void* VR_CALLTYPE VRChaperoneSetup();
VR_INTERFACE void* VR_CALLTYPE VRCompositor();
VR_INTERFACE void* VR_CALLTYPE VROverlay();
VR_INTERFACE void* VR_CALLTYPE VRRenderModels();
VR_INTERFACE void* VR_CALLTYPE VRApplications();
VR_INTERFACE void* VR_CALLTYPE VRSettings();
VR_INTERFACE void* VR_CALLTYPE VRExtendedDisplay();
VR_INTERFACE void* VR_CALLTYPE VRTrackedCamera();
VR_INTERFACE void* VR_CALLTYPE VRNotifications();

inline void* VR_CALLTYPE VRSystem() { return OpenVR_v0_9_12::OpenVRInternal_ModuleContext().VRSystem(); }
inline void* VR_CALLTYPE VRChaperone() { return OpenVR_v0_9_12::OpenVRInternal_ModuleContext().VRChaperone(); }
inline void* VR_CALLTYPE VRChaperoneSetup() { return OpenVR_v0_9_12::OpenVRInternal_ModuleContext().VRChaperoneSetup(); }
inline void* VR_CALLTYPE VRCompositor() { return OpenVR_v0_9_12::OpenVRInternal_ModuleContext().VRCompositor(); }
inline void* VR_CALLTYPE VROverlay() { return OpenVR_v0_9_12::OpenVRInternal_ModuleContext().VROverlay(); }
inline void* VR_CALLTYPE VRRenderModels() { return OpenVR_v0_9_12::OpenVRInternal_ModuleContext().VRRenderModels(); }
inline void* VR_CALLTYPE VRApplications() { return OpenVR_v0_9_12::OpenVRInternal_ModuleContext().VRApplications(); }
inline void* VR_CALLTYPE VRSettings() { return OpenVR_v0_9_12::OpenVRInternal_ModuleContext().VRSettings(); }
inline void* VR_CALLTYPE VRExtendedDisplay() { return OpenVR_v0_9_12::OpenVRInternal_ModuleContext().VRExtendedDisplay(); }
inline void* VR_CALLTYPE VRTrackedCamera() { return OpenVR_v0_9_12::OpenVRInternal_ModuleContext().VRTrackedCamera(); }
inline void* VR_CALLTYPE VRNotifications() { return OpenVR_v0_9_12::OpenVRInternal_ModuleContext().VRNotifications(); }

// VR_INTERFACE void* VR_CALLTYPE VR_Init(vr::EVRInitError* peError, vr::EVRApplicationType eApplicationType, const char* pStartupInfo);
/** Finds the active installation of vrclient.dll and initializes it */
VR_INTERFACE void* VR_CALLTYPE VR_Init(EVRInitError* peError, EVRApplicationType eApplicationType)
{
	void* pVRSystem = nullptr;

	EVRInitError eError;
	OpenVR_v0_9_12::VRToken() = VR_InitInternal2(&eError, eApplicationType, NULL);
	OpenVR_v0_9_12::COpenVRContext& ctx = OpenVR_v0_9_12::OpenVRInternal_ModuleContext();
	ctx.Clear();

	if (eError == VRInitError_None) {
		if (VR_IsInterfaceVersionValid(OpenVR_v0_9_12::IVRSystem_Version)) {
			pVRSystem = VRSystem();
		} else {
			VR_ShutdownInternal();
			eError = VRInitError_Init_InterfaceNotFound;
		}
	}

	if (peError)
		*peError = eError;

	return pVRSystem;
}

/** unloads vrclient.dll. Any interface pointers from the interface are
 * invalid after this point */
VR_INTERFACE void VR_CALLTYPE VR_Shutdown()
{
	VR_ShutdownInternal();
}

#endif
