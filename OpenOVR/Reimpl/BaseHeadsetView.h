#pragma once

#include <cstdint>

enum OOVR_HeadsetViewMode_t {
	HeadsetViewMode_Left = 0,
	HeadsetViewMode_Right,
	HeadsetViewMode_Both
};

class BaseHeadsetView {
public:
	/** Sets the resolution in pixels to render the headset view. These values are clamped to k_unHeadsetViewMaxWidth
	 * and k_unHeadsetViewMaxHeight respectively. For cropped views, the rendered output will be fit to aspect ratio
	 * defined by the the specified dimensions. For uncropped views, the caller should use GetHeadsetViewAspectRation
	 * to adjust the requested render size to avoid squashing or stretching, and then apply letterboxing to compensate
	 * when displaying the results. */
	virtual void SetHeadsetViewSize(uint32_t nWidth, uint32_t nHeight);

	/** Gets the current resolution used to render the headset view. */
	virtual void GetHeadsetViewSize(uint32_t* pnWidth, uint32_t* pnHeight);

	/** Set the mode used to render the headset view. */
	virtual void SetHeadsetViewMode(OOVR_HeadsetViewMode_t eHeadsetViewMode);

	/** Get the current mode used to render the headset view. */
	virtual OOVR_HeadsetViewMode_t GetHeadsetViewMode();

	/** Set whether or not the headset view should be rendered cropped to hide the hidden area mesh or not. */
	virtual void SetHeadsetViewCropped(bool bCropped);

	/** Get the current cropping status of the headset view. */
	virtual bool GetHeadsetViewCropped();

	/** Get the aspect ratio (width:height) of the uncropped headset view (accounting for the current set mode). */
	virtual float GetHeadsetViewAspectRatio();

	/** Set the range [0..1] that the headset view blends across the stereo overlapped area in cropped both mode. */
	virtual void SetHeadsetViewBlendRange(float flStartPct, float flEndPct);

	/** Get the current range [0..1] that the headset view blends across the stereo overlapped area in cropped both mode. */
	virtual void GetHeadsetViewBlendRange(float* pStartPct, float* pEndPct);
};
