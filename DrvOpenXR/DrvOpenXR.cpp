//
// Created by ZNix on 25/10/2020.
//

#include "DrvOpenXR.h"

#include "../OpenOVR/Misc/Config.h"
#include "../OpenOVR/Misc/android_api.h"
#include "../OpenOVR/Misc/xr_ext.h"
#include "../OpenOVR/Reimpl/BaseInput.h"
#include "XrBackend.h"
#include "generated/static_bases.gen.h"
#include "tmp_gfx/TemporaryGraphics.h"

#if defined(SUPPORT_DX) && defined(SUPPORT_DX11)
#include "tmp_gfx/TemporaryD3D11.h"
#endif

#if defined(SUPPORT_VK)
#include "tmp_gfx/TemporaryVk.h"
#endif

#include <memory>
#include <set>
#include <string>

static XrBackend* currentBackend;
static bool initialised = false;
static std::unique_ptr<TemporaryGraphics> temporaryGraphics;

#ifdef _WIN32
std::string GetExeName()
{
	char exePath[MAX_PATH + 1] = { 0 };
	DWORD len = GetModuleFileNameA(NULL, exePath, MAX_PATH);
	PathStripPathA(exePath);
	return { exePath };
}
#else
#include <libgen.h> // basename
#include <linux/limits.h> // PATH_MAX
#include <unistd.h> // readlink

std::string GetExeName()
{
	char exePath[PATH_MAX + 1] = { 0 };
	ssize_t count = readlink("/proc/self/exe", exePath, PATH_MAX);
	if (count != -1) {
		return { basename(exePath) };
	}
	return { "" };
}
#endif

void DrvOpenXR::GetXRAppName(char (&appName)[128])
{
	std::string exeName = GetExeName();
	if (exeName.size() > 0) {
		std::string ocAppName{ "OpenComposite_" };
		ocAppName.append(exeName);
		auto pos = ocAppName.find(".exe");
		if (pos != std::string::npos && pos > 0)
			ocAppName = ocAppName.substr(0, pos);
		OOVR_LOGF("Setting application name to %s", ocAppName.c_str());
		strcpy_arr(appName, ocAppName.c_str());
	} else {
		strcpy_arr(appName, "OpenComposite");
	}
}

#ifdef _DEBUG
static XrDebugUtilsMessengerEXT dbgMessenger = NULL;

static XrBool32 XRAPI_CALL debugCallback(
    XrDebugUtilsMessageSeverityFlagsEXT messageSeverity,
    XrDebugUtilsMessageTypeFlagsEXT messageTypes,
    const XrDebugUtilsMessengerCallbackDataEXT* callbackData,
    void* userData)
{
	OOVR_LOGF("debugCallback %s", callbackData->message);
	return XR_FALSE;
}
#endif

IBackend* DrvOpenXR::CreateOpenXRBackend()
{
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

	// Enumerate the available extensions
	uint32_t availableExtensionsCount;
	OOVR_FAILED_XR_ABORT(xrEnumerateInstanceExtensionProperties(nullptr, 0, &availableExtensionsCount, nullptr));
	std::vector<XrExtensionProperties> extensionProperties;
	extensionProperties.resize(availableExtensionsCount, { XR_TYPE_EXTENSION_PROPERTIES });
	OOVR_FAILED_XR_ABORT(xrEnumerateInstanceExtensionProperties(nullptr,
	    extensionProperties.size(), &availableExtensionsCount, extensionProperties.data()));
	std::set<std::string> availableExtensions;
	for (const XrExtensionProperties& ext : extensionProperties) {
		availableExtensions.insert(ext.extensionName);
		OOVR_LOGF("Extension: %s", ext.extensionName);
	}

	uint32_t availableLayersCount;
	OOVR_FAILED_XR_ABORT(xrEnumerateApiLayerProperties(0, &availableLayersCount, nullptr));
	OOVR_LOGF("Num layers available: %d ", availableLayersCount);
	std::vector<XrApiLayerProperties> layerProperties;
	layerProperties.resize(availableLayersCount, { XR_TYPE_API_LAYER_PROPERTIES });
	OOVR_FAILED_XR_ABORT(xrEnumerateApiLayerProperties(
	    layerProperties.size(), &availableLayersCount, layerProperties.data()));
	std::set<std::string> availableLayers;
	for (const XrApiLayerProperties& layer : layerProperties) {
		availableLayers.insert(layer.layerName);
		OOVR_LOGF("Layer: %s", layer.layerName);
	}

	// Create the OpenXR instance - this is the overall handle that connects us to the runtime
	// https://www.khronos.org/registry/OpenXR/specs/1.0/refguide/openxr-10-reference-guide.pdf
	XrApplicationInfo appInfo{};
	GetXRAppName(appInfo.applicationName);
	appInfo.applicationVersion = 1;
	appInfo.apiVersion = XR_CURRENT_API_VERSION;

	std::vector<const char*> extensions;
	XrGraphicsApiSupportedFlags apiFlags = 0;

#if defined(SUPPORT_DX) && defined(SUPPORT_DX11)
	if (availableExtensions.count("XR_KHR_D3D11_enable")) {
		extensions.push_back("XR_KHR_D3D11_enable");
		apiFlags |= XR_SUPPORTED_GRAPHCIS_API_D3D11;
	}
#endif
#if defined(SUPPORT_DX) && defined(SUPPORT_DX12)
	if (availableExtensions.count("XR_KHR_D3D12_enable")) {
		extensions.push_back("XR_KHR_D3D12_enable");
		apiFlags |= XR_SUPPORTED_GRAPHCIS_API_D3D12;
	}
#endif
#if defined(SUPPORT_VK)
	if (availableExtensions.count(XR_KHR_VULKAN_ENABLE_EXTENSION_NAME)) {
		extensions.push_back(XR_KHR_VULKAN_ENABLE_EXTENSION_NAME);
		apiFlags |= XR_SUPPORTED_GRAPHCIS_API_VK;
	}
#endif
#if defined(SUPPORT_GL)
	if (availableExtensions.count(XR_KHR_OPENGL_ENABLE_EXTENSION_NAME)) {
		extensions.push_back(XR_KHR_OPENGL_ENABLE_EXTENSION_NAME);
		apiFlags |= XR_SUPPORTED_GRAPHCIS_API_GL;
	}
#endif
#if defined(SUPPORT_GLES)
	if (availableExtensions.count(XR_KHR_OPENGL_ES_ENABLE_EXTENSION_NAME)) {
		extensions.push_back(XR_KHR_OPENGL_ES_ENABLE_EXTENSION_NAME);
		apiFlags |= OC_SUPPORTED_GRAPHCIS_API_GLES;
	}
#endif
#if defined(ANDROID)
	if (availableExtensions.count(XR_KHR_ANDROID_CREATE_INSTANCE_EXTENSION_NAME))
		extensions.push_back(XR_KHR_ANDROID_CREATE_INSTANCE_EXTENSION_NAME);
#endif
	if (availableExtensions.count(XR_EXT_DEBUG_UTILS_EXTENSION_NAME))
		extensions.push_back(XR_EXT_DEBUG_UTILS_EXTENSION_NAME);

	// If the visibility mask is available use it, otherwise no big deal
	if (availableExtensions.count(XR_KHR_VISIBILITY_MASK_EXTENSION_NAME))
		extensions.push_back(XR_KHR_VISIBILITY_MASK_EXTENSION_NAME);

	if (availableExtensions.count(XR_EXT_HAND_TRACKING_EXTENSION_NAME))
		extensions.push_back(XR_EXT_HAND_TRACKING_EXTENSION_NAME);

	const char* const layers[] = {
#ifdef XR_VALIDATION_LAYER_PATH
		"XR_APILAYER_LUNARG_core_validation",
#endif
		nullptr // Dummy value since MSVC gets upset if there's nothing in this array
	};

	XrInstanceCreateInfo createInfo{};
	createInfo.type = XR_TYPE_INSTANCE_CREATE_INFO;
	createInfo.applicationInfo = appInfo;
	createInfo.enabledExtensionNames = extensions.data();
	createInfo.enabledExtensionCount = extensions.size();
	createInfo.enabledApiLayerNames = layers;
	createInfo.enabledApiLayerCount = (sizeof(layers) / sizeof(const char*)) - 1; // Subtract the dummy value

#if ANDROID
	if (!OpenComposite_Android_Create_Info) {
		OOVR_ABORT("Cannot create OpenXR instance - OpenComposite_Android_Create_Info not set");
	}
	XrInstanceCreateInfoAndroidKHR androidInfo = *OpenComposite_Android_Create_Info;
	androidInfo.next = nullptr;
	createInfo.next = &androidInfo;
#endif

	OOVR_FAILED_XR_ABORT(xrCreateInstance(&createInfo, &xr_instance));

#ifdef _DEBUG
	XrDebugUtilsMessengerCreateInfoEXT dbgCreateInfo{};
	dbgCreateInfo.type = XR_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	dbgCreateInfo.messageSeverities = XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	dbgCreateInfo.messageTypes = XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_TYPE_CONFORMANCE_BIT_EXT;
	dbgCreateInfo.next = NULL;
	dbgCreateInfo.userData = NULL;
	if (dbgMessenger != NULL) {
		OOVR_FAILED_XR_ABORT(xrDestroyDebugUtilsMessengerEXT(dbgMessenger));
		dbgMessenger = NULL;
	}
	dbgCreateInfo.userCallback = debugCallback;
	OOVR_FAILED_XR_ABORT(xrCreateDebugUtilsMessengerEXT(xr_instance, &dbgCreateInfo, &dbgMessenger));
#endif

	// Load the function pointers for the extension functions
	xr_ext = new XrExt(apiFlags, extensions);
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
#if defined(SUPPORT_VK)
		if ((apiFlags & XR_SUPPORTED_GRAPHCIS_API_VK) && oovr_global_configuration.InitUsingVulkan()) {
			temporaryGraphics = std::make_unique<TemporaryVk>();
		}
#endif

#if defined(SUPPORT_DX) && defined(SUPPORT_DX11)
		if (!temporaryGraphics && (apiFlags & XR_SUPPORTED_GRAPHCIS_API_D3D11)) {
			temporaryGraphics = std::make_unique<TemporaryD3D11>();
		}
#endif

#if !defined(SUPPORT_VK) && !defined(SUPPORT_DX) && !defined(SUPPORT_DX11)
#error No available temporary graphics implementation
#endif

		OOVR_FALSE_ABORT(temporaryGraphics);
	}

	// Build a backend that works with OpenXR
	currentBackend = new XrBackend();

	// FIXME HACK HACK HACK hardcode D3D11 now, since xrCreateSession returns XR_ERROR_GRAPHICS_DEVICE_INVALID if
	//  you don't pass it a graphics binding. Unfortunately we don't know what graphics API the game is using (much
	//  less have a handle to it) until it submits it's first frame.
	SetupSession(temporaryGraphics->GetGraphicsBinding());

	return currentBackend;
}

void DrvOpenXR::SetupSession(const void* graphicsBinding)
{
	if (xr_gbl) {
		ShutdownSession();
	}

	if (temporaryGraphics && graphicsBinding != temporaryGraphics->GetGraphicsBinding())
		temporaryGraphics.reset();

	XrSessionCreateInfo sessionInfo{};
	sessionInfo.type = XR_TYPE_SESSION_CREATE_INFO;
	sessionInfo.systemId = xr_system;
	sessionInfo.next = graphicsBinding;
	OOVR_FAILED_XR_ABORT(xrCreateSession(xr_instance, &sessionInfo, &xr_session));

	// Setup the OpenXR globals, which uses the current session so we have to do this last
	xr_gbl = new XrSessionGlobals();

	// Print the current version for diagnostic purposes
	OOVR_LOGF("Started OpenXR session on runtime '%s', hand tracking supported: %d",
	    xr_gbl->systemProperties.systemName, xr_gbl->handTrackingProperties.supportsHandTracking);

	// If required, re-setup the input system for this new session
	BaseInput* input = GetUnsafeBaseInput();
	if (input)
		input->BindInputsForSession();

	currentBackend->OnSessionCreated();
}

void DrvOpenXR::ShutdownSession()
{
	BackendManager* instance = BackendManager::InstancePtr();
	// Is it already being shut down?
	// Note that this is indirectly called by the XrBackend destructor, which will have
	// already called this function in that case.
	if (instance) {
		auto* backend = (XrBackend*)instance->GetBackendInstance();
		if (backend)
			backend->PrepareForSessionShutdown();
	}

	delete xr_gbl;
	xr_gbl = nullptr;

	// Hey it turns out that xrDestroySession can be called whenever - how convenient
	// OOVR_FAILED_XR_ABORT(xrRequestExitSession(xr_session));

	OOVR_FAILED_XR_ABORT(xrDestroySession(xr_session));
	xr_session = XR_NULL_HANDLE;

	// Destroy the graphics after destroying the session, otherwise when the runtime goes to destroy it's swapchain
	// it will be sad, in a probably SEGFAULT-y way.
	temporaryGraphics.reset();
}

void DrvOpenXR::FullShutdown()
{
	if (xr_session)
		ShutdownSession();

#ifdef _DEBUG
	if (dbgMessenger != NULL) {
		OOVR_FAILED_XR_ABORT(xrDestroyDebugUtilsMessengerEXT(dbgMessenger));
		dbgMessenger = NULL;
	}
#endif

	if (xr_instance) {
		OOVR_FAILED_XR_ABORT(xrDestroyInstance(xr_instance));
		xr_instance = XR_NULL_HANDLE;
	}

	initialised = false;
	currentBackend = nullptr;
}

#ifdef SUPPORT_VK
void DrvOpenXR::VkGetPhysicalDevice(VkInstance instance, VkPhysicalDevice* out)
{
	*out = VK_NULL_HANDLE;

	TemporaryVk* vk = temporaryGraphics->GetAsVk();
	if (vk == nullptr)
		OOVR_ABORT("Not using temporary Vulkan instance");

	// Find the UUID of the physical device the temporary instance is running on
	VkPhysicalDeviceIDProperties idProps = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES };
	VkPhysicalDeviceProperties2 props = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, &idProps };
	vkGetPhysicalDeviceProperties2(vk->physicalDevice, &props);

	// Look through all the physical devices on the target instance and find the matching one
	uint32_t devCount;
	OOVR_FAILED_VK_ABORT(vkEnumeratePhysicalDevices(instance, &devCount, nullptr));
	std::vector<VkPhysicalDevice> physicalDevices(devCount);
	OOVR_FAILED_VK_ABORT(vkEnumeratePhysicalDevices(instance, &devCount, physicalDevices.data()));

	for (VkPhysicalDevice phy : physicalDevices) {
		VkPhysicalDeviceIDProperties devIdProps = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES };
		VkPhysicalDeviceProperties2 devProps = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, &devIdProps };
		vkGetPhysicalDeviceProperties2(phy, &devProps);

		if (memcmp(devIdProps.deviceUUID, idProps.deviceUUID, sizeof(devIdProps.deviceUUID)) != 0)
			continue;

		// Found it
		*out = phy;
		return;
	}

	OOVR_ABORT("Could not find matching Vulkan physical device for instance");
}

#endif
