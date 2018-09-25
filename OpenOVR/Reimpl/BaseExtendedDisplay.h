#pragma once
#include "BaseCommon.h"

class BaseExtendedDisplay {
public:

	/** Size and position that the window needs to be on the VR display. */
	virtual void GetWindowBounds(int32_t *pnX, int32_t *pnY, uint32_t *pnWidth, uint32_t *pnHeight);

	/** Gets the viewport in the frame buffer to draw the output of the distortion into */
	virtual void GetEyeOutputViewport(vr::EVREye eEye, uint32_t *pnX, uint32_t *pnY, uint32_t *pnWidth, uint32_t *pnHeight);

	/** [D3D10/11 Only]
	* Returns the adapter index and output index that the user should pass into EnumAdapters and EnumOutputs
	* to create the device and swap chain in DX10 and DX11. If an error occurs both indices will be set to -1.
	*/
	virtual void GetDXGIOutputInfo(int32_t *pnAdapterIndex, int32_t *pnAdapterOutputIndex);

};
