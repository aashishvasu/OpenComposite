#pragma once
#include "../OpenOVR/Drivers/Backend.h"

namespace DrvOpenXR {
IBackend* CreateOpenXRBackend();
#ifdef SUPPORT_VK
void VkGetPhysicalDevice(VkInstance instance, VkPhysicalDevice* out);
class TemporaryVk* GetTemporaryVk();
#endif
}; // namespace DrvOpenXR
