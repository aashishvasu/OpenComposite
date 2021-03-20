#pragma once

#include "compositor.h"
#include "glcompositor.h"

class GLESCompositor : public GLBaseCompositor {
public:
	explicit GLESCompositor();

protected:
	void ReadSwapchainImages() override;
};
