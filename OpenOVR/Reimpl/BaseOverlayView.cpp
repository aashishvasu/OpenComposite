#include "stdafx.h"
#define BASE_IMPL
#include "BaseOverlayView.h"

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
vr::EVROverlayError BaseOverlayView::AcquireOverlayView(vr::VROverlayHandle_t ulOverlayHandle,
		OOVR_VRNativeDevice_t *pNativeDevice, OOVR_VROverlayView_t *pOverlayView, uint32_t unOverlayViewSize) {
	STUBBED();
}

/** Release an acquired OverlayView_t
*
* Denotes that pOverlayView will no longer require access to the resources it acquired in
* all previous calls to AcquireOverlayView().
*
* All OverlayView_t*'s provided to AcquireOverlayView() as pOverlayViews must be
* passed into ReleaseOverlayView() in order for the underlying GPU resources to be freed.
*/
vr::EVROverlayError BaseOverlayView::ReleaseOverlayView(OOVR_VROverlayView_t *pOverlayView) {
	STUBBED();
}

/** Posts an overlay event */
void BaseOverlayView::PostOverlayEvent(vr::VROverlayHandle_t ulOverlayHandle, const vr::VREvent_t *pvrEvent) {
	STUBBED();
}

/** Determines whether this process is permitted to view an overlay's content. */
bool BaseOverlayView::IsViewingPermitted(vr::VROverlayHandle_t ulOverlayHandle) {
	STUBBED();
}
