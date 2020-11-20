//
// Created by ZNix on 25/10/2020.
//

#include "DrvOpenXR.h"

#include "../OpenOVR/Misc/xr_ext.h"
#include "XrBackend.h"

#include <string>

static ID3D11Device* CreateTemporaryD3D11Device();

IBackend* DrvOpenXR::CreateOpenXRBackend()
{
	static bool initialised = false;

	// TODO handle something like Unity which stops and restarts the instance
	if (initialised) {
		OOVR_ABORT("Cannot double-initialise OpenXR");
	}

	// FIXME should be removed, for testing only
	OOVR_FALSE_ABORT(SetEnvironmentVariableA("XR_CORE_VALIDATION_EXPORT_TYPE", "text"));
	OOVR_FALSE_ABORT(SetEnvironmentVariableA("XR_API_LAYER_PATH",
	    "C:\\Users\\ZNix\\source\\repos\\OpenCompositeXR\\build-openxr\\src\\api_layers"))
	OOVR_FALSE_ABORT(SetEnvironmentVariableA("XR_CORE_VALIDATION_FILE_NAME",
	    "C:\\Users\\ZNix\\source\\repos\\OpenCompositeXR\\validation-xr.log"))

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
		"XR_EXT_debug_utils",
	};

	const char* const layers[] = {
		"XR_APILAYER_LUNARG_core_validation",
	};

	XrInstanceCreateInfo createInfo{};
	createInfo.type = XR_TYPE_INSTANCE_CREATE_INFO;
	createInfo.applicationInfo = appInfo;
	createInfo.enabledExtensionNames = extensions;
	createInfo.enabledExtensionCount = sizeof(extensions) / sizeof(const char*);
	createInfo.enabledApiLayerNames = layers;
	createInfo.enabledApiLayerCount = sizeof(layers) / sizeof(const char*);

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

	// FIXME HACK HACK HACK hardcode D3D11 now, since xrCreateSession returns XR_ERROR_GRAPHICS_DEVICE_INVALID if
	//  you don't pass it a graphics binding. Unfortunately we don't know what graphics API the game is using (much
	//  less have a handle to it) until it submits it's first frame.
	// FIXME I'm only using it here as a POC since it's easier to set up than Vulkan, but we'll need to switch over
	//  to that later on in the name of compatibility.
	XrGraphicsBindingD3D11KHR d3dInfo{};
	d3dInfo.type = XR_TYPE_GRAPHICS_BINDING_D3D11_KHR;
	d3dInfo.device = CreateTemporaryD3D11Device(); // Nooooo

	SetupSession(&d3dInfo);

	// Build a backend that works with OpenXR
	return new XrBackend();
}

void DrvOpenXR::SetupSession(void* graphicsBinding)
{
	if (xr_gbl) {
		ShutdownSession();
	}

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
	delete xr_gbl;
	xr_gbl = nullptr;

	// Hey it turns out that xrDestroySession can be called whenever - how convenient
	// OOVR_FAILED_XR_ABORT(xrRequestExitSession(xr_session));
	// // Like with xrBeginSession, we're really not supposed to call this until xrRequestExitSession is done
	// OOVR_FAILED_XR_ABORT(xrEndSession(xr_session));

	OOVR_FAILED_XR_ABORT(xrDestroySession(xr_session));
	xr_session = nullptr;
}

/**
 * One big hack as detailed above for session creation.
 *
 * Yes this leaves a D3D device, no I don't care for now.
 */
static ID3D11Device* CreateTemporaryD3D11Device()
{
	// The spec requires that we call this first, and use it to get the correct things
	XrGraphicsRequirementsD3D11KHR graphicsRequirements{};
	graphicsRequirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR;
	XrResult res = xr_ext->xrGetD3D11GraphicsRequirementsKHR(xr_instance, xr_system, &graphicsRequirements);
	OOVR_FAILED_XR_ABORT(res);

	// TODO use the proper adapter
	IDXGIAdapter* adapter = nullptr;
	ID3D11Device* dev = nullptr;

	// Such a horrid hack - of all the ugly things we do in OpenComposite, this has to be one of the worst.
	HRESULT createDeviceRes = D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
	    &graphicsRequirements.minFeatureLevel, 1,
	    D3D11_SDK_VERSION, &dev, nullptr, nullptr);
	OOVR_FAILED_DX_ABORT(createDeviceRes);

	return dev;
}
