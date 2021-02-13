//
// Created by ZNix on 8/02/2021.
//

#ifdef SUPPORT_VK

#include "TemporaryVk.h"

// TODO move this to the VK compositor maybe?
static void parseExtensionsStr(std::string extensionsStr, std::vector<std::string>& store, std::vector<const char*>& cList)
{
	OOVR_LOGF("Str: %s", extensionsStr.data());
	while (true) {
		size_t firstSpace = extensionsStr.find_first_of(' ');
		if (firstSpace == std::string::npos) {
			store.push_back(extensionsStr);
			break;
		}

		std::string subStr = extensionsStr.substr(0, firstSpace);
		store.push_back(subStr);
		extensionsStr.erase(0, firstSpace + 1);
	}

	cList.reserve(store.size());
	for (const std::string& s : store) {
		cList.push_back(s.c_str());
		OOVR_LOGF("Add extension: %s", s.c_str());
	}
}

TemporaryVk::TemporaryVk()
{
	// Call because we have to, TODO check we have a supported Vulkan version
	XrGraphicsRequirementsVulkanKHR req = { XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR };
	OOVR_FAILED_XR_ABORT(xr_ext->xrGetVulkanGraphicsRequirementsKHR(xr_instance, xr_system, &req));

	// Create a Vulkan instance that the runtime will find suitable
	uint32_t extCap;
	OOVR_FAILED_XR_ABORT(xr_ext->xrGetVulkanInstanceExtensionsKHR(xr_instance, xr_system, 0, &extCap, nullptr));
	std::string xrInstanceExtensions;
	xrInstanceExtensions.resize(extCap, 0);
	OOVR_FAILED_XR_ABORT(xr_ext->xrGetVulkanInstanceExtensionsKHR(xr_instance, xr_system, xrInstanceExtensions.size(), &extCap, (char*)xrInstanceExtensions.data()));

	std::vector<std::string> instanceExtensionsStore;
	std::vector<const char*> instanceExtensionsCstr;
	parseExtensionsStr(xrInstanceExtensions, instanceExtensionsStore, instanceExtensionsCstr);

	VkInstanceCreateInfo createInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	createInfo.enabledExtensionCount = instanceExtensionsCstr.size();
	createInfo.ppEnabledExtensionNames = instanceExtensionsCstr.data();
	// Don't use any layers here

	OOVR_FAILED_VK_ABORT(vkCreateInstance(&createInfo, nullptr, &instance));

	// Find the physical device we're supposed to use
	VkPhysicalDevice physicalDev;
	OOVR_FAILED_XR_ABORT(xr_ext->xrGetVulkanGraphicsDeviceKHR(xr_instance, xr_system, instance, &physicalDev));

	// Find a suitable queue type on that device
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDev, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilyProps(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDev, &queueFamilyCount, queueFamilyProps.data());

	OOVR_FALSE_ABORT(queueFamilyCount > 0); // Just use the first queue type
	int queueFamilyIdx = 0;

	// Prepare the queue creation info
	float priority = 1.0f;
	VkDeviceQueueCreateInfo queueInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
	queueInfo.queueFamilyIndex = queueFamilyIdx;
	queueInfo.queueCount = 1;
	queueInfo.pQueuePriorities = &priority;

	// Now create the logical device
	OOVR_FAILED_XR_ABORT(xr_ext->xrGetVulkanDeviceExtensionsKHR(xr_instance, xr_system, 0, &extCap, nullptr));
	std::string deviceExtensionsStr;
	deviceExtensionsStr.resize(extCap, 0);
	OOVR_FAILED_XR_ABORT(xr_ext->xrGetVulkanDeviceExtensionsKHR(xr_instance, xr_system, deviceExtensionsStr.size(), &extCap, (char*)deviceExtensionsStr.data()));

	std::vector<std::string> deviceExtensionsStore;
	std::vector<const char*> deviceExtensionsCstr;
	parseExtensionsStr(deviceExtensionsStr, deviceExtensionsStore, deviceExtensionsCstr);

	VkDeviceCreateInfo devCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	devCreateInfo.enabledExtensionCount = deviceExtensionsCstr.size();
	devCreateInfo.ppEnabledExtensionNames = deviceExtensionsCstr.data();

	devCreateInfo.queueCreateInfoCount = 1;
	devCreateInfo.pQueueCreateInfos = &queueInfo;

	OOVR_FAILED_VK_ABORT(vkCreateDevice(physicalDev, &devCreateInfo, nullptr, &device));

	// Setup our graphics binding
	binding = XrGraphicsBindingVulkanKHR{ XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR };
	binding.instance = instance;
	binding.physicalDevice = physicalDev;
	binding.device = device;
	binding.queueFamilyIndex = queueFamilyIdx;
	binding.queueIndex = 0;
}

TemporaryVk::~TemporaryVk()
{
	vkDestroyDevice(device, nullptr);
	device = nullptr;

	vkDestroyInstance(instance, nullptr);
	instance = nullptr;
}

#endif
