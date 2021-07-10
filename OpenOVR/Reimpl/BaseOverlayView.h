#pragma once
#include "../BaseCommon.h"

struct OOVR_VROverlayView_t
{
	vr::VROverlayHandle_t overlayHandle;
	vr::Texture_t texture;
	vr::VRTextureBounds_t textureBounds;
};

enum OOVR_EDeviceType
{
	DeviceType_Invalid = -1, // Invalid handle
	DeviceType_DirectX11 = 0, // Handle is an ID3D11Device
	DeviceType_Vulkan = 1, // Handle is a pointer to a VRVulkanDevice_t structure
};

struct OOVR_VRVulkanDevice_t
{
	VkInstance_T *m_pInstance;
	VkDevice_T *m_pDevice;
	VkPhysicalDevice_T *m_pPhysicalDevice;
	VkQueue_T *m_pQueue;
	uint32_t m_uQueueFamilyIndex;
};

struct OOVR_VRNativeDevice_t
{
	void *handle; // See EDeviceType definition above
	OOVR_EDeviceType eType;
};

class BaseOverlayView
{
public:
	/** Acquire an OverlayView_t from an overlay handle
	*
	* The overlay view can be used to sample the contents directly by a native API. The
	* contents of the OverlayView_t will remain unchanged through the lifetime of the
	* OverlayView_t.
	*
	* The caller acquires read privileges over the OverlayView_t, but should not
	* write to it.
	*
	* AcquireOverlayView() may be called on the same ulOverlayHandle multiple times to
	* refresh the image contents. In this case the caller is strongly encouraged to re-use
	* the same pOverlayView for all re-acquisition calls.
	*
	* If the producer has not yet queued an image, AcquireOverlayView will return success,
	* and the Texture_t will have the expected ETextureType. However, the Texture_t->handle
	* will be nullptr. Once the producer generates the first overlay frame, Texture_t->handle
	* will become a valid handle.
	*/
	virtual vr::EVROverlayError AcquireOverlayView(vr::VROverlayHandle_t ulOverlayHandle,
			OOVR_VRNativeDevice_t *pNativeDevice, OOVR_VROverlayView_t *pOverlayView, uint32_t unOverlayViewSize);

	/** Release an acquired OverlayView_t
	*
	* Denotes that pOverlayView will no longer require access to the resources it acquired in
	* all previous calls to AcquireOverlayView().
	*
	* All OverlayView_t*'s provided to AcquireOverlayView() as pOverlayViews must be
	* passed into ReleaseOverlayView() in order for the underlying GPU resources to be freed.
	*/
	virtual vr::EVROverlayError ReleaseOverlayView(OOVR_VROverlayView_t *pOverlayView);

	/** Posts an overlay event */
	virtual void PostOverlayEvent(vr::VROverlayHandle_t ulOverlayHandle, const vr::VREvent_t *pvrEvent);

	/** Determines whether this process is permitted to view an overlay's content. */
	virtual bool IsViewingPermitted(vr::VROverlayHandle_t ulOverlayHandle);
};
