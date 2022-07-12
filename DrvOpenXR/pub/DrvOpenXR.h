#pragma once
#include "../OpenOVR/Drivers/Backend.h"

// Since we're not importing stdafx, add another nasty little hack
#ifndef _WIN32
#include "../OpenOVR/linux_funcs.h"
#endif

namespace DrvOpenXR {
IBackend* CreateOpenXRBackend();
void GetXRAppName(char (&appName)[128]);
#ifdef SUPPORT_VK
// No
void VkGetPhysicalDevice(VkInstance instance, VkPhysicalDevice* out);
// class TemporaryVk* GetTemporaryVk();
#endif
}; // namespace DrvOpenXR
