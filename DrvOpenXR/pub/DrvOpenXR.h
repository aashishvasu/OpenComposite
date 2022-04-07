#pragma once
#include "../OpenOVR/Drivers/Backend.h"

namespace DrvOpenXR {
IBackend* CreateOpenXRBackend();
void GetXRAppName(char (& appName)[128]);
#ifdef SUPPORT_VK
void VkGetPhysicalDevice(VkInstance instance, VkPhysicalDevice* out);
class TemporaryVk* GetTemporaryVk();
#endif
}; // namespace DrvOpenXR
