//
// Created by ZNix on 25/10/2020.
//

#include "DrvOpenXR.h"

#include "../OpenOVR/Misc/xr_ext.h"
#include "XrBackend.h"
#include "tmp_gfx/TemporaryGraphics.h"

#if defined(SUPPORT_DX) && defined(SUPPORT_DX11)
#include "tmp_gfx/TemporaryD3D11.h"
#endif

#if defined(SUPPORT_VK)
#include "tmp_gfx/TemporaryVk.h"
#endif

#include <memory>
#include <string>

static std::unique_ptr<TemporaryGraphics> temporaryGraphics;

IBackend* DrvOpenXR::CreateOpenXRBackend()
{
	static bool initialised = false;

	// TODO handle something like Unity which stops and restarts the instance
	if (initialised) {
		OOVR_ABORT("Cannot double-initialise OpenXR");
	}
	initialised = true;

	// TODO make these work on Linux
#ifdef XR_VALIDATION_LAYER_PATH
	OOVR_FALSE_ABORT(SetEnvironmentVariableA("XR_CORE_VALIDATION_EXPORT_TYPE", "text"));
	OOVR_FALSE_ABORT(SetEnvironmentVariableA("XR_API_LAYER_PATH", XR_VALIDATION_LAYER_PATH));
	OOVR_LOGF("Set OpenXR Layer path: %s", XR_VALIDATION_LAYER_PATH);
#endif
#ifdef XR_VALIDATION_FILE_NAME
	OOVR_FALSE_ABORT(SetEnvironmentVariableA("XR_CORE_VALIDATION_FILE_NAME", XR_VALIDATION_FILE_NAME));
	OOVR_LOGF("Set OpenXR validation file path: %s", XR_VALIDATION_FILE_NAME);
#endif

	// Create the OpenXR instance - this is the overall handle that connects us to the runtime
	// https://www.khronos.org/registry/OpenXR/specs/1.0/refguide/openxr-10-reference-guide.pdf
	XrApplicationInfo appInfo{};
	strcpy(appInfo.applicationName, "OpenComposite"); // TODO vary by application
	appInfo.applicationVersion = 1;
	appInfo.apiVersion = XR_CURRENT_API_VERSION;

	const char* const extensions[] = {
#ifdef SUPPORT_DX
		"XR_KHR_D3D11_enable",
#endif
#if defined(SUPPORT_VK)
		"XR_KHR_vulkan_enable",
#endif
		"XR_EXT_debug_utils",
		"XR_KHR_visibility_mask",
	};

	const char* const layers[] = {
#ifdef XR_VALIDATION_LAYER_PATH
		"XR_APILAYER_LUNARG_core_validation",
#endif
		nullptr // Dummy value since MSVC gets upset if there's nothing in this array
	};

	XrInstanceCreateInfo createInfo{};
	createInfo.type = XR_TYPE_INSTANCE_CREATE_INFO;
	createInfo.applicationInfo = appInfo;
	createInfo.enabledExtensionNames = extensions;
	createInfo.enabledExtensionCount = sizeof(extensions) / sizeof(const char*);
	createInfo.enabledApiLayerNames = layers;
	createInfo.enabledApiLayerCount = (sizeof(layers) / sizeof(const char*)) - 1; // Subtract the dummy value

	OOVR_FAILED_XR_ABORT(xrCreateInstance(&createInfo, &xr_instance));

	// Load the function pointers for the extension functions
	xr_ext = new XrExt();

	// Create a system - this is when we choose what form factor we want, in this case an HMD
	XrSystemGetInfo systemInfo{};
	systemInfo.type = XR_TYPE_SYSTEM_GET_INFO;
	systemInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
	OOVR_FAILED_XR_ABORT(xrGetSystem(xr_instance, &systemInfo, &xr_system));

	// List off the views and store them locally for easy access
	uint32_t viewCount = 0;
	OOVR_FAILED_XR_ABORT(xrEnumerateViewConfigurationViews(xr_instance, xr_system,
	    XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, 0, &viewCount, nullptr));

	xr_views_list = std::vector<XrViewConfigurationView>(viewCount, { XR_TYPE_VIEW_CONFIGURATION_VIEW });

	OOVR_FAILED_XR_ABORT(xrEnumerateViewConfigurationViews(xr_instance, xr_system, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
	    viewCount, &viewCount, xr_views_list.data()));

	OOVR_FALSE_ABORT(viewCount == xr_views_list.size());

	// Create a session - this tells the runtime that sooner or later we'd like to submit frames
	// This is when we have to choose what graphics API to use

	if (!temporaryGraphics) {
#if defined(SUPPORT_DX) && defined(SUPPORT_DX11)
		temporaryGraphics = std::make_unique<TemporaryD3D11>();
#elif defined(SUPPORT_VK)
		temporaryGraphics = std::make_unique<TemporaryVk>();
#else
#error No available temporary graphics implementation
#endif
	}

	// FIXME HACK HACK HACK hardcode D3D11 now, since xrCreateSession returns XR_ERROR_GRAPHICS_DEVICE_INVALID if
	//  you don't pass it a graphics binding. Unfortunately we don't know what graphics API the game is using (much
	//  less have a handle to it) until it submits it's first frame.
	SetupSession(temporaryGraphics->GetGraphicsBinding());

	// Build a backend that works with OpenXR
	return new XrBackend();
}

void DrvOpenXR::SetupSession(const void* graphicsBinding)
{
	if (xr_gbl) {
		ShutdownSession();
	}

	if (graphicsBinding != temporaryGraphics->GetGraphicsBinding())
		temporaryGraphics.reset();

	XrSessionCreateInfo sessionInfo{};
	sessionInfo.type = XR_TYPE_SESSION_CREATE_INFO;
	sessionInfo.systemId = xr_system;
	sessionInfo.next = graphicsBinding;
	OOVR_FAILED_XR_ABORT(xrCreateSession(xr_instance, &sessionInfo, &xr_session));

	// Start the session running
	// FIXME we're not supposed to do this right away, and instead listen for an XrEventDataSessionStateChanged event
	// FIXME this only works on the Oculus runtime due to an implementation detail, and isn't portable
	XrSessionBeginInfo beginInfo{ XR_TYPE_SESSION_BEGIN_INFO };
	beginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
	OOVR_FAILED_XR_ABORT(xrBeginSession(xr_session, &beginInfo));

	// Setup the OpenXR globals, which uses the current session so we have to do this last
	xr_gbl = new XrSessionGlobals();
}

void DrvOpenXR::ShutdownSession()
{
	temporaryGraphics.reset();

	delete xr_gbl;
	xr_gbl = nullptr;

	// Hey it turns out that xrDestroySession can be called whenever - how convenient
	// OOVR_FAILED_XR_ABORT(xrRequestExitSession(xr_session));
	// // Like with xrBeginSession, we're really not supposed to call this until xrRequestExitSession is done
	// OOVR_FAILED_XR_ABORT(xrEndSession(xr_session));

	OOVR_FAILED_XR_ABORT(xrDestroySession(xr_session));
	xr_session = XR_NULL_HANDLE;
}

void DrvOpenXR::FullShutdown()
{
	if (xr_session)
		ShutdownSession();

	if (xr_instance) {
		OOVR_FAILED_XR_ABORT(xrDestroyInstance(xr_instance));
		xr_instance = XR_NULL_HANDLE;
	}
}

#ifdef SUPPORT_VK
TemporaryVk* DrvOpenXR::GetTemporaryVk()
{
	return temporaryGraphics->GetAsVk();
}
#endif
