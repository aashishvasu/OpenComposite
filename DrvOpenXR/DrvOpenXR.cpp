//
// Created by ZNix on 25/10/2020.
//

#include "DrvOpenXR.h"

#include "../OpenOVR/Misc/xr_ext.h"
#include "../OpenOVR/Reimpl/BaseInput.h"
#include "../OpenOVR/Reimpl/static_bases.gen.h"
#include "XrBackend.h"
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

// The application must fill this out
#if ANDROID
extern "C" XrInstanceCreateInfoAndroidKHR* OpenComposite_Android_Create_Info = nullptr;
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
	}

	// Create the OpenXR instance - this is the overall handle that connects us to the runtime
	// https://www.khronos.org/registry/OpenXR/specs/1.0/refguide/openxr-10-reference-guide.pdf
	XrApplicationInfo appInfo{};
	strcpy(appInfo.applicationName, "OpenComposite"); // TODO vary by application
	appInfo.applicationVersion = 1;
	appInfo.apiVersion = XR_CURRENT_API_VERSION;

	std::vector<const char*> extensions;
#ifdef SUPPORT_DX
	extensions.push_back("XR_KHR_D3D11_enable");
#endif
#if defined(SUPPORT_VK)
	extensions.push_back(XR_KHR_VULKAN_ENABLE_EXTENSION_NAME);
#endif
#if defined(SUPPORT_GL)
	extensions.push_back(XR_KHR_OPENGL_ENABLE_EXTENSION_NAME);
#endif
#if defined(ANDROID)
	extensions.push_back(XR_KHR_ANDROID_CREATE_INSTANCE_EXTENSION_NAME);
#endif
	extensions.push_back(XR_EXT_DEBUG_UTILS_EXTENSION_NAME);

	// If the visibility mask is available use it, otherwise no big deal
	if (availableExtensions.count(XR_KHR_VISIBILITY_MASK_EXTENSION_NAME))
		extensions.push_back(XR_KHR_VISIBILITY_MASK_EXTENSION_NAME);

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
#if defined(SUPPORT_VK)
		// If we have Vulkan prioritise that, since we need it if the application uses Vulkan
		temporaryGraphics = std::make_unique<TemporaryVk>();
#elif defined(SUPPORT_DX) && defined(SUPPORT_DX11)
		temporaryGraphics = std::make_unique<TemporaryD3D11>();
#else
#error No available temporary graphics implementation
#endif
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

	// If required, re-setup the input system for this new session
	BaseInput* input = GetUnsafeBaseInput();
	if (input)
		input->BindInputsForSession();

	currentBackend->OnSessionCreated();
}

void DrvOpenXR::ShutdownSession()
{
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

	TemporaryVk* vk = GetTemporaryVk();
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

TemporaryVk* DrvOpenXR::GetTemporaryVk()
{
	return temporaryGraphics->GetAsVk();
}
#endif
