//
// Created by ZNix on 8/02/2021.
//

#pragma once

#include "TemporaryGraphics.h"

#include "../XrDriverPrivate.h"

class TemporaryVk : public TemporaryGraphics {
public:
	TemporaryVk();
	~TemporaryVk() override;

	const void* GetGraphicsBinding() const override { return &binding; }

	TemporaryVk* GetAsVk() override { return this; }

private:
	XrGraphicsBindingVulkanKHR binding = {};

public:
	VkInstance instance = VK_NULL_HANDLE;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	VkQueue queue = VK_NULL_HANDLE;
};
