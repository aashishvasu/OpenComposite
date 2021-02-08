//
// Created by ZNix on 8/02/2021.
//

#if defined(SUPPORT_DX11)

#include "TemporaryD3D11.h"

TemporaryD3D11::TemporaryD3D11()
{
	// The spec requires that we call this first, and use it to get the correct things
	XrGraphicsRequirementsD3D11KHR graphicsRequirements{};
	graphicsRequirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR;
	XrResult res = xr_ext->xrGetD3D11GraphicsRequirementsKHR(xr_instance, xr_system, &graphicsRequirements);
	OOVR_FAILED_XR_ABORT(res);

	// TODO use the proper adapter
	IDXGIAdapter* adapter = nullptr;

	// Such a horrid hack - of all the ugly things we do in OpenComposite, this has to be one of the worst.
	HRESULT createDeviceRes = D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
	    &graphicsRequirements.minFeatureLevel, 1,
	    D3D11_SDK_VERSION, &device, nullptr, nullptr);

	OOVR_FAILED_DX_ABORT(createDeviceRes);

	d3dInfo = XrGraphicsBindingD3D11KHR{ XR_TYPE_GRAPHICS_BINDING_D3D11_KHR };
	d3dInfo.device = device;
}

TemporaryD3D11::~TemporaryD3D11()
{
	device->Release();
}

#endif
