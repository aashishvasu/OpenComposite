//
// Created by ZNix on 8/02/2021.
//

#pragma once

#include "TemporaryGraphics.h"

#include "../XrDriverPrivate.h"

class TemporaryD3D11 : public TemporaryGraphics {
public:
	TemporaryD3D11();
	~TemporaryD3D11() override;

	const void* GetGraphicsBinding() const override { return &d3dInfo; }

private:
	ID3D11Device* device = nullptr;
	XrGraphicsBindingD3D11KHR d3dInfo = {};
};
