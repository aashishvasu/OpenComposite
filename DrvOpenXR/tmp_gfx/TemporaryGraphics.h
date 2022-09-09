//
// Created by ZNix on 8/02/2021.
//

#pragma once

/**
 * A class to encapsulate the temporary graphics instance. Please see DrvOpenXR::CreateOpenVRBackend for
 * the details about this horrid little hack.
 */
class TemporaryGraphics {
public:
	virtual ~TemporaryGraphics() = default;

	virtual const void* GetGraphicsBinding() const = 0;

	virtual class TemporaryVk* GetAsVk() { return nullptr; }
};
