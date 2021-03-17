#include "stdafx.h"
#define BASE_IMPL
#include "BaseOverlay.h"
#include "BaseSystem.h"
#include <string>
#include "Compositor/compositor.h"
#include "convert.h"
#include "BaseCompositor.h"
#include "static_bases.gen.h"
#include "Misc/Config.h"
#include "Misc/ScopeGuard.h"

using namespace std;
using glm::vec3;
using glm::mat4;

// Class to represent an overlay
class BaseOverlay::OverlayData {
public:
	const string key;
	string name;
	HmdColor_t colour;

	float widthMeters = 1; // default 1 meter

	float autoCurveDistanceRangeMin, autoCurveDistanceRangeMax; // WTF does this do?
	EColorSpace colourSpace = ColorSpace_Auto;
	bool visible = false; // TODO check against SteamVR
	VRTextureBounds_t textureBounds = { 0, 0, 1, 1 };
	VROverlayInputMethod inputMethod = VROverlayInputMethod_None; // TODO fire events
	HmdVector2_t mouseScale = { 1.0f, 1.0f };
	bool highQuality = false;
	uint64_t flags = 0;
	float texelAspect = 1;
	std::queue<VREvent_t> eventQueue;

	// Rendering
	Texture_t texture = {};
#ifndef OC_XR_PORT
	ovrLayerQuad layerQuad = {};
#endif
	std::unique_ptr<Compositor> compositor;

	// Transform
	VROverlayTransformType transformType = VROverlayTransform_Absolute;
	union {
		struct {
			HmdMatrix34_t offset;
			TrackedDeviceIndex_t  device;
		} deviceRelative;
	} transformData;

	OverlayData(string key, string name) : key(key), name(name) {
	}
};

// TODO don't pass around handles, as it will cause
// crashes when we should merely return VROverlayError_InvalidHandle
#define OVL (*((OverlayData**)pOverlayHandle))
#define USEH() \
OverlayData *overlay = (OverlayData*)ulOverlayHandle; \
if (!overlay || !validOverlays.count(overlay) || !overlays.count(overlay->key)) { \
    return VROverlayError_InvalidHandle; \
}

#define USEHB() \
OverlayData *overlay = (OverlayData*)ulOverlayHandle; \
if (!overlay || !overlays.count(overlay->key)) { \
	return false; \
}

BaseOverlay::~BaseOverlay() {
	for (const auto &kv : overlays) {
		if (kv.second) {
			delete kv.second;
		}
	}
}

int BaseOverlay::_BuildLayers(ovrLayerHeader_ * sceneLayer, ovrLayerHeader_ const* const*& layers) {
	// Note that at least on MSVC, this shouldn't be doing any memory allocations
	//  unless the list is expanding from new layers.
	layerHeaders.clear();
	layerHeaders.push_back(sceneLayer);

	// For performance reasons, this indicates that something is using input
	// Don't write it directly to usingInput, as that could cause race conditions between
	//  when it's initially set to false, and when it's reset to true.
	bool checkUsingInput = false;

	if (keyboard) {
#ifndef OC_XR_PORT
		layerHeaders.push_back(keyboard->Update());
#endif
		checkUsingInput = true;

		if (keyboard->IsClosed())
			HideKeyboard();
	}

	if (!oovr_global_configuration.EnableLayers()) {
		goto done;
	}

	for (const auto &kv : overlays) {
		if (kv.second) {
			OverlayData &overlay = *kv.second;

			// Skip hiddden overlays, and those without a valid texture (eg, after calling ClearOverlayTexture).
			if (!overlay.visible || overlay.texture.handle == nullptr)
				continue;

			// Quick hack to get around Boneworks creating overlays and setting them to an opacity of
			// zero to hide them. Leave 1% of margin in case of weird float issues.
			if (overlay.colour.a < 0.01)
				continue;

#ifndef OC_XR_PORT
			// Calculate the texture's aspect ratio
			const ovrSizei &srcSize = overlay.layerQuad.Viewport.Size;
			const float aspect = srcSize.h > 0 ? static_cast<float>(srcSize.w) / srcSize.h : 1.0f;

			// ... and use that to set the size of the overlay, as it will appear to the user
			// Note we shouldn't do this when setting the texture, as the user may change the width of
			//  the overlay without changing the texture.
			overlay.layerQuad.QuadSize.x = overlay.widthMeters;
			overlay.layerQuad.QuadSize.y = overlay.widthMeters / aspect;

			if (overlay.transformType == VROverlayTransform_TrackedDeviceRelative) {
				TrackedDeviceIndex_t dev = overlay.transformData.deviceRelative.device;

				OVR::Matrix4f offset;
				S2O_om44(overlay.transformData.deviceRelative.offset, offset);

				TrackedDevicePose_t pose{};

				BaseCompositor *comp = GetUnsafeBaseCompositor();
				BaseSystem *sys = GetUnsafeBaseSystem();
				if (comp && sys) {
					comp->GetSinglePoseRendering(sys->_GetRenderTrackingOrigin(), dev, &pose);
				}

				if (pose.bPoseIsValid) {
					OVR::Matrix4f devOffset;
					S2O_om44(pose.mDeviceToAbsoluteTracking, devOffset);

					offset = devOffset * offset;
				}

				overlay.layerQuad.QuadPoseCenter = OVR::Posef(OVR::Quatf(offset), offset.GetTranslation());
			}

			// Finally, add it to the list of layers to be sent to LibOVR
			layerHeaders.push_back(&overlay.layerQuad.Header);
#endif
		}
	}

done:
	usingInput = checkUsingInput;
	layers = layerHeaders.data();
	return static_cast<int>(layerHeaders.size());
}

bool BaseOverlay::_HandleOverlayInput(EVREye side, TrackedDeviceIndex_t index, VRControllerState_t state) {
	if (!usingInput)
		return true;

	if (!keyboard)
		return true;

#ifndef OC_XR_PORT
	keyboard->HandleOverlayInput(side, state, (float)ovr_GetTimeInSeconds());
#endif
	return false;
}

EVROverlayError BaseOverlay::FindOverlay(const char *pchOverlayKey, VROverlayHandle_t * pOverlayHandle) {
	if (overlays.count(pchOverlayKey)) {
		OVL = overlays[pchOverlayKey];
		return VROverlayError_None;
	}

	// TODO is this the correct return value
	return VROverlayError_InvalidParameter;
}
EVROverlayError BaseOverlay::CreateOverlay(const char *pchOverlayKey, const char *pchOverlayName, VROverlayHandle_t * pOverlayHandle) {
	if (overlays.count(pchOverlayKey)) {
		return VROverlayError_KeyInUse;
	}

	OverlayData *data = new OverlayData(pchOverlayKey, pchOverlayName);
	OVL = data;

	overlays[pchOverlayKey] = data;
	validOverlays.insert(data);

#ifndef OC_XR_PORT
	// Set up the LibOVR layer
	OOVR_LOGF(R"(New texture overlay created "%s" "%s")", pchOverlayKey, pchOverlayName);
	data->layerQuad.Header.Type = ovrLayerType_Quad;
	data->layerQuad.Header.Flags = ovrLayerFlag_HighQuality;

	// 50cm in front from the player's nose by default, in case the game does not set the position
	//  before it first uses it.
	data->layerQuad.QuadPoseCenter.Position.x = 0.00f;
	data->layerQuad.QuadPoseCenter.Position.y = 0.00f;
	data->layerQuad.QuadPoseCenter.Position.z = -0.50f;
	data->layerQuad.QuadPoseCenter.Orientation.x = 0;
	data->layerQuad.QuadPoseCenter.Orientation.y = 0;
	data->layerQuad.QuadPoseCenter.Orientation.z = 0;
	data->layerQuad.QuadPoseCenter.Orientation.w = 1;

	// Note we don't need to set the layer QuadSize, as this is set before the frame is submitted

	// Contents texture starts at 0,0 - this is not overridden
	data->layerQuad.Viewport.Pos.x = 0;
	data->layerQuad.Viewport.Pos.y = 0;
#endif

	return VROverlayError_None;
}
EVROverlayError BaseOverlay::DestroyOverlay(VROverlayHandle_t ulOverlayHandle) {
	USEH();

	if (highQualityOverlay == ulOverlayHandle)
		highQualityOverlay = NULL;

	overlays.erase(overlay->key);
	validOverlays.erase(overlay);
	delete overlay;

	return VROverlayError_None;
}
EVROverlayError BaseOverlay::SetHighQualityOverlay(VROverlayHandle_t ulOverlayHandle) {
	USEH();

	highQualityOverlay = ulOverlayHandle;

	return VROverlayError_None;
}
VROverlayHandle_t BaseOverlay::GetHighQualityOverlay() {
	if (!highQualityOverlay)
		return k_ulOverlayHandleInvalid;

	return highQualityOverlay;
}
uint32_t BaseOverlay::GetOverlayKey(VROverlayHandle_t ulOverlayHandle, char *pchValue, uint32_t unBufferSize, EVROverlayError *pError) {
	OverlayData *overlay = (OverlayData*)ulOverlayHandle;
	if (!overlays.count(overlay->key)) {
		if (pError)
			*pError = VROverlayError_InvalidHandle;
		if (unBufferSize != 0)
			pchValue[0] = NULL;
		return 0;
	}

	const char *key = overlay->key.c_str();
	strncpy_s(pchValue, unBufferSize, key, unBufferSize);

	if (strlen(key) >= unBufferSize && unBufferSize != 0) {
		pchValue[unBufferSize - 1] = 0;
	}

	if(pError)
		*pError = VROverlayError_None;

	// Is this supposed to include the NULL or not?
	// TODO test, this could cause some very nasty bugs
	return static_cast<uint32_t>(strlen(pchValue) + 1);
}
uint32_t BaseOverlay::GetOverlayName(VROverlayHandle_t ulOverlayHandle, VR_OUT_STRING() char *pchValue, uint32_t unBufferSize, EVROverlayError *pError) {
	if (pError)
		*pError = VROverlayError_None;

	OverlayData *overlay = (OverlayData*)ulOverlayHandle;
	if (!overlays.count(overlay->key)) {
		if (pError)
			*pError = VROverlayError_InvalidHandle;
		if (unBufferSize != 0)
			pchValue[0] = NULL;
		return 0;
	}

	const char *name = overlay->name.c_str();
	strncpy_s(pchValue, unBufferSize, name, unBufferSize);

	if (strlen(name) >= unBufferSize && unBufferSize != 0) {
		pchValue[unBufferSize - 1] = 0;
	}

	// Is this supposed to include the NULL or not?
	// TODO test, this could cause some very nasty bugs
	return static_cast<uint32_t>(strlen(pchValue) + 1);
}
EVROverlayError BaseOverlay::SetOverlayName(VROverlayHandle_t ulOverlayHandle, const char *pchName) {
	USEH();

	overlay->name = pchName;

	return VROverlayError_None;
}
EVROverlayError BaseOverlay::GetOverlayImageData(VROverlayHandle_t ulOverlayHandle, void *pvBuffer, uint32_t unBufferSize, uint32_t *punWidth, uint32_t *punHeight) {
	STUBBED();
}
const char * BaseOverlay::GetOverlayErrorNameFromEnum(EVROverlayError error) {
#define ERR_CASE(name) case VROverlayError_ ## name: return #name;
	switch (error) {
		ERR_CASE(None);
		ERR_CASE(UnknownOverlay);
		ERR_CASE(InvalidHandle);
		ERR_CASE(PermissionDenied);
		ERR_CASE(OverlayLimitExceeded);
		ERR_CASE(WrongVisibilityType);
		ERR_CASE(KeyTooLong);
		ERR_CASE(NameTooLong);
		ERR_CASE(KeyInUse);
		ERR_CASE(WrongTransformType);
		ERR_CASE(InvalidTrackedDevice);
		ERR_CASE(InvalidParameter);
		ERR_CASE(ThumbnailCantBeDestroyed);
		ERR_CASE(ArrayTooSmall);
		ERR_CASE(RequestFailed);
		ERR_CASE(InvalidTexture);
		ERR_CASE(UnableToLoadFile);
		ERR_CASE(KeyboardAlreadyInUse);
		ERR_CASE(NoNeighbor);
		ERR_CASE(TooManyMaskPrimitives);
		ERR_CASE(BadMaskPrimitive);
	}
#undef ERR_CASE

	string msg = "Unknown overlay error code: " + to_string(error);
	OOVR_LOG(msg.c_str());

	STUBBED();
}
EVROverlayError BaseOverlay::SetOverlayRenderingPid(VROverlayHandle_t ulOverlayHandle, uint32_t unPID) {
	STUBBED();
}
uint32_t BaseOverlay::GetOverlayRenderingPid(VROverlayHandle_t ulOverlayHandle) {
	STUBBED();
}
EVROverlayError BaseOverlay::SetOverlayFlag(VROverlayHandle_t ulOverlayHandle, VROverlayFlags eOverlayFlag, bool bEnabled) {
	USEH();

	if (bEnabled) {
		overlay->flags |= 1uLL << eOverlayFlag;
	}
	else {
		overlay->flags &= ~(1uLL << eOverlayFlag);
	}

	return VROverlayError_None;
}
EVROverlayError BaseOverlay::GetOverlayFlag(VROverlayHandle_t ulOverlayHandle, VROverlayFlags eOverlayFlag, bool *pbEnabled) {
	USEH();

	*pbEnabled = (overlay->flags & (1uLL << eOverlayFlag)) != 0uLL;

	return VROverlayError_None;
}
EVROverlayError BaseOverlay::SetOverlayColor(VROverlayHandle_t ulOverlayHandle, float fRed, float fGreen, float fBlue) {
	USEH();

	overlay->colour.r = fRed;
	overlay->colour.g = fGreen;
	overlay->colour.b = fBlue;

	return VROverlayError_None;
}
EVROverlayError BaseOverlay::GetOverlayColor(VROverlayHandle_t ulOverlayHandle, float *pfRed, float *pfGreen, float *pfBlue) {
	USEH();

	*pfRed = overlay->colour.r;
	*pfGreen = overlay->colour.g;
	*pfBlue = overlay->colour.b;

	return VROverlayError_None;
}
EVROverlayError BaseOverlay::SetOverlayAlpha(VROverlayHandle_t ulOverlayHandle, float fAlpha) {
	USEH();

	overlay->colour.a = fAlpha;

	return VROverlayError_None;
}
EVROverlayError BaseOverlay::GetOverlayAlpha(VROverlayHandle_t ulOverlayHandle, float *pfAlpha) {
	USEH();

	*pfAlpha = overlay->colour.a;

	return VROverlayError_None;
}
EVROverlayError BaseOverlay::SetOverlayTexelAspect(VROverlayHandle_t ulOverlayHandle, float fTexelAspect) {
	USEH();

	overlay->texelAspect = fTexelAspect;

	return VROverlayError_None;
}
EVROverlayError BaseOverlay::GetOverlayTexelAspect(VROverlayHandle_t ulOverlayHandle, float *pfTexelAspect) {
	USEH();

	if (!pfTexelAspect)
		OOVR_ABORT("pfTexelAspect == nullptr");

	*pfTexelAspect = overlay->texelAspect;

	return VROverlayError_None;
}
EVROverlayError BaseOverlay::SetOverlaySortOrder(VROverlayHandle_t ulOverlayHandle, uint32_t unSortOrder) {
	// TODO
	return VROverlayError_None;
}
EVROverlayError BaseOverlay::GetOverlaySortOrder(VROverlayHandle_t ulOverlayHandle, uint32_t *punSortOrder) {
	STUBBED();
}
EVROverlayError BaseOverlay::SetOverlayWidthInMeters(VROverlayHandle_t ulOverlayHandle, float fWidthInMeters) {
	USEH();

	overlay->widthMeters = fWidthInMeters;

	return VROverlayError_None;
}
EVROverlayError BaseOverlay::GetOverlayWidthInMeters(VROverlayHandle_t ulOverlayHandle, float *pfWidthInMeters) {
	USEH();

	*pfWidthInMeters = overlay->widthMeters;

	return VROverlayError_None;
}
EVROverlayError BaseOverlay::SetOverlayCurvature(VROverlayHandle_t ulOverlayHandle, float fCurvature) {
	STUBBED();
}
EVROverlayError BaseOverlay::GetOverlayCurvature(VROverlayHandle_t ulOverlayHandle, float *pfCurvature) {
	STUBBED();
}
EVROverlayError BaseOverlay::SetOverlayAutoCurveDistanceRangeInMeters(VROverlayHandle_t ulOverlayHandle, float fMinDistanceInMeters, float fMaxDistanceInMeters) {
	USEH();

	overlay->autoCurveDistanceRangeMin = fMinDistanceInMeters;
	overlay->autoCurveDistanceRangeMax = fMaxDistanceInMeters;

	return VROverlayError_None;
}
EVROverlayError BaseOverlay::GetOverlayAutoCurveDistanceRangeInMeters(VROverlayHandle_t ulOverlayHandle, float *pfMinDistanceInMeters, float *pfMaxDistanceInMeters) {
	USEH();

	*pfMinDistanceInMeters = overlay->autoCurveDistanceRangeMin;
	*pfMaxDistanceInMeters = overlay->autoCurveDistanceRangeMax;

	return VROverlayError_None;
}
EVROverlayError BaseOverlay::SetOverlayTextureColorSpace(VROverlayHandle_t ulOverlayHandle, EColorSpace eTextureColorSpace) {
	USEH();

	overlay->colourSpace = eTextureColorSpace;

	return VROverlayError_None;
}
EVROverlayError BaseOverlay::GetOverlayTextureColorSpace(VROverlayHandle_t ulOverlayHandle, EColorSpace *peTextureColorSpace) {
	USEH();

	*peTextureColorSpace = overlay->colourSpace;

	return VROverlayError_None;
}
EVROverlayError BaseOverlay::SetOverlayTextureBounds(VROverlayHandle_t ulOverlayHandle, const VRTextureBounds_t *pOverlayTextureBounds) {
	USEH();

	if(pOverlayTextureBounds)
		overlay->textureBounds = *pOverlayTextureBounds;
	else
		overlay->textureBounds = { 0, 0, 1, 1 };

	return VROverlayError_None;
}
EVROverlayError BaseOverlay::GetOverlayTextureBounds(VROverlayHandle_t ulOverlayHandle, VRTextureBounds_t *pOverlayTextureBounds) {
	USEH();

	*pOverlayTextureBounds = overlay->textureBounds;

	return VROverlayError_None;
}
uint32_t BaseOverlay::GetOverlayRenderModel(VROverlayHandle_t ulOverlayHandle, char *pchValue, uint32_t unBufferSize, HmdColor_t *pColor, EVROverlayError *pError) {
	if (pError)
		*pError = VROverlayError_None;

	STUBBED();
}
EVROverlayError BaseOverlay::SetOverlayRenderModel(VROverlayHandle_t ulOverlayHandle, const char *pchRenderModel, const HmdColor_t *pColor) {
	STUBBED();
}
EVROverlayError BaseOverlay::GetOverlayTransformType(VROverlayHandle_t ulOverlayHandle, VROverlayTransformType *peTransformType) {
	USEH();

	*peTransformType = overlay->transformType;

	return VROverlayError_None;
}
EVROverlayError BaseOverlay::SetOverlayTransformAbsolute(VROverlayHandle_t ulOverlayHandle, ETrackingUniverseOrigin eTrackingOrigin, const HmdMatrix34_t *pmatTrackingOriginToOverlayTransform) {
	USEH();

	// TODO account for the universe origin, and if it doesn't match that currently in use then add or
	//  subtract the floor position to match it. This shouldn't usually be an issue though, as I can't
	//  imagine many apps will use a different origin for their overlays.

	overlay->transformType = VROverlayTransform_Absolute;
#ifndef OC_XR_PORT
	overlay->layerQuad.QuadPoseCenter = S2O_om34_pose(*pmatTrackingOriginToOverlayTransform);
#endif

	return VROverlayError_None;
}
EVROverlayError BaseOverlay::GetOverlayTransformAbsolute(VROverlayHandle_t ulOverlayHandle, ETrackingUniverseOrigin *peTrackingOrigin, HmdMatrix34_t *pmatTrackingOriginToOverlayTransform) {
	STUBBED();
}
EVROverlayError BaseOverlay::SetOverlayTransformTrackedDeviceRelative(VROverlayHandle_t ulOverlayHandle, TrackedDeviceIndex_t unTrackedDevice, const HmdMatrix34_t *pmatTrackedDeviceToOverlayTransform) {
	USEH();

	overlay->transformType = VROverlayTransform_TrackedDeviceRelative;
	overlay->transformData.deviceRelative.device = unTrackedDevice;
	overlay->transformData.deviceRelative.offset = *pmatTrackedDeviceToOverlayTransform;

	return VROverlayError_None;
}
EVROverlayError BaseOverlay::GetOverlayTransformTrackedDeviceRelative(VROverlayHandle_t ulOverlayHandle, TrackedDeviceIndex_t *punTrackedDevice, HmdMatrix34_t *pmatTrackedDeviceToOverlayTransform) {
	STUBBED();
}
EVROverlayError BaseOverlay::SetOverlayTransformTrackedDeviceComponent(VROverlayHandle_t ulOverlayHandle, TrackedDeviceIndex_t unDeviceIndex, const char *pchComponentName) {
	STUBBED();
}
EVROverlayError BaseOverlay::GetOverlayTransformTrackedDeviceComponent(VROverlayHandle_t ulOverlayHandle, TrackedDeviceIndex_t *punDeviceIndex, char *pchComponentName, uint32_t unComponentNameSize) {
	STUBBED();
}
EVROverlayError BaseOverlay::GetOverlayTransformOverlayRelative(VROverlayHandle_t ulOverlayHandle, VROverlayHandle_t *ulOverlayHandleParent, HmdMatrix34_t *pmatParentOverlayToOverlayTransform) {
	STUBBED();
}
EVROverlayError BaseOverlay::SetOverlayTransformOverlayRelative(VROverlayHandle_t ulOverlayHandle, VROverlayHandle_t ulOverlayHandleParent, const HmdMatrix34_t *pmatParentOverlayToOverlayTransform) {
	// TODO
	return VROverlayError_None;
}
EVROverlayError BaseOverlay::SetOverlayTransformCursor(VROverlayHandle_t ulCursorOverlayHandle, const HmdVector2_t *pvHotspot) {
	STUBBED();
}
EVROverlayError BaseOverlay::GetOverlayTransformCursor(VROverlayHandle_t ulOverlayHandle, HmdVector2_t *pvHotspot) {
	STUBBED();
}
EVROverlayError BaseOverlay::SetOverlayTransformProjection(VROverlayHandle_t ulOverlayHandle,
    ETrackingUniverseOrigin eTrackingOrigin, const HmdMatrix34_t* pmatTrackingOriginToOverlayTransform,
    const OOVR_VROverlayProjection_t* pProjection, EVREye eEye)
{
	STUBBED();
}
EVROverlayError BaseOverlay::ShowOverlay(VROverlayHandle_t ulOverlayHandle) {
	USEH();
	overlay->visible = true;
	return VROverlayError_None;
}
EVROverlayError BaseOverlay::HideOverlay(VROverlayHandle_t ulOverlayHandle) {
	USEH();
	overlay->visible = false;
	return VROverlayError_None;
}
bool BaseOverlay::IsOverlayVisible(VROverlayHandle_t ulOverlayHandle) {
	USEHB();
	return overlay->visible;
}
EVROverlayError BaseOverlay::GetTransformForOverlayCoordinates(VROverlayHandle_t ulOverlayHandle, ETrackingUniverseOrigin eTrackingOrigin, HmdVector2_t coordinatesInOverlay, HmdMatrix34_t *pmatTransform) {
	STUBBED();
}
bool BaseOverlay::PollNextOverlayEvent(VROverlayHandle_t ulOverlayHandle, VREvent_t *pEvent, uint32_t eventSize) {
	USEHB();

	memset(pEvent, 0, eventSize);

	if (overlay->eventQueue.empty())
		return false;

	VREvent_t e = overlay->eventQueue.front();
	overlay->eventQueue.pop();

	memcpy(pEvent, &e, min((uint32_t)sizeof(e), eventSize));

	return true;
}
EVROverlayError BaseOverlay::GetOverlayInputMethod(VROverlayHandle_t ulOverlayHandle, VROverlayInputMethod *peInputMethod) {
	USEH();

	if (peInputMethod)
		*peInputMethod = overlay->inputMethod;

	return VROverlayError_None;
}

EVROverlayError BaseOverlay::SetOverlayInputMethod(VROverlayHandle_t ulOverlayHandle, VROverlayInputMethod eInputMethod) {
	USEH();

	overlay->inputMethod = eInputMethod;

	return VROverlayError_None;
}
EVROverlayError BaseOverlay::GetOverlayMouseScale(VROverlayHandle_t ulOverlayHandle, HmdVector2_t *pvecMouseScale) {
	USEH();

	*pvecMouseScale = overlay->mouseScale;

	return VROverlayError_None;
}
EVROverlayError BaseOverlay::SetOverlayMouseScale(VROverlayHandle_t ulOverlayHandle, const HmdVector2_t *pvecMouseScale) {
	USEH();

	if (pvecMouseScale)
		overlay->mouseScale = *pvecMouseScale;
	else
		overlay->mouseScale = HmdVector2_t{ 1.0f, 1.0f };

	return VROverlayError_None;
}
bool BaseOverlay::ComputeOverlayIntersection(VROverlayHandle_t ulOverlayHandle, const OOVR_VROverlayIntersectionParams_t *pParams, OOVR_VROverlayIntersectionResults_t *pResults) {
	STUBBED();
}
bool BaseOverlay::HandleControllerOverlayInteractionAsMouse(VROverlayHandle_t ulOverlayHandle, TrackedDeviceIndex_t unControllerDeviceIndex) {
	STUBBED();
}
bool BaseOverlay::IsHoverTargetOverlay(VROverlayHandle_t ulOverlayHandle) {
	STUBBED();
}
VROverlayHandle_t BaseOverlay::GetGamepadFocusOverlay() {
	STUBBED();
}
EVROverlayError BaseOverlay::SetGamepadFocusOverlay(VROverlayHandle_t ulNewFocusOverlay) {
	STUBBED();
}
EVROverlayError BaseOverlay::SetOverlayNeighbor(EOverlayDirection eDirection, VROverlayHandle_t ulFrom, VROverlayHandle_t ulTo) {
	STUBBED();
}
EVROverlayError BaseOverlay::MoveGamepadFocusToNeighbor(EOverlayDirection eDirection, VROverlayHandle_t ulFrom) {
	STUBBED();
}
EVROverlayError BaseOverlay::SetOverlayDualAnalogTransform(VROverlayHandle_t ulOverlay, EDualAnalogWhich eWhich, const HmdVector2_t & vCenter, float fRadius) {
	STUBBED();
}
EVROverlayError BaseOverlay::GetOverlayDualAnalogTransform(VROverlayHandle_t ulOverlay, EDualAnalogWhich eWhich, HmdVector2_t *pvCenter, float *pfRadius) {
	STUBBED();
}
EVROverlayError BaseOverlay::SetOverlayDualAnalogTransform(VROverlayHandle_t ulOverlay, EDualAnalogWhich eWhich, const HmdVector2_t *pvCenter, float fRadius) {
	STUBBED();
}
EVROverlayError BaseOverlay::TriggerLaserMouseHapticVibration(VROverlayHandle_t ulOverlayHandle, float fDurationSeconds, float fFrequency, float fAmplitude) {
	STUBBED();
}
EVROverlayError BaseOverlay::SetOverlayCursor(VROverlayHandle_t ulOverlayHandle, VROverlayHandle_t ulCursorHandle) {
	STUBBED();
}
EVROverlayError BaseOverlay::SetOverlayCursorPositionOverride(VROverlayHandle_t ulOverlayHandle, const HmdVector2_t *pvCursor) {
	STUBBED();
}
EVROverlayError BaseOverlay::ClearOverlayCursorPositionOverride(VROverlayHandle_t ulOverlayHandle) {
	STUBBED();
}
EVROverlayError BaseOverlay::SetOverlayTexture(VROverlayHandle_t ulOverlayHandle, const Texture_t *pTexture) {
	USEH();
	overlay->texture = *pTexture;

	if (!oovr_global_configuration.EnableLayers())
		return VROverlayError_None;

#ifndef OC_XR_PORT
	if (!overlay->compositor) {
		const auto size = ovr_GetFovTextureSize(*ovr::session, ovrEye_Left, ovr::hmdDesc.DefaultEyeFov[ovrEye_Left], 1);
		overlay->compositor.reset(GetUnsafeBaseCompositor()->CreateCompositorAPI(pTexture, size));
	}

	overlay->compositor->LoadSubmitContext();
	auto revertToCallerContext = MakeScopeGuard([&]() {
		overlay->compositor->ResetSubmitContext();
	});

	overlay->compositor->Invoke(&overlay->texture);

	overlay->layerQuad.Viewport.Size = overlay->compositor->GetSrcSize();
	overlay->layerQuad.ColorTexture = overlay->compositor->GetSwapChain();

	OOVR_FAILED_OVR_ABORT(ovr_CommitTextureSwapChain(*ovr::session, overlay->layerQuad.ColorTexture));
#endif

	return VROverlayError_None;
}
EVROverlayError BaseOverlay::ClearOverlayTexture(VROverlayHandle_t ulOverlayHandle) {
	USEH();
	overlay->texture = {};

	overlay->compositor.reset();
	return VROverlayError_None;
}
EVROverlayError BaseOverlay::SetOverlayRaw(VROverlayHandle_t ulOverlayHandle, void *pvBuffer, uint32_t unWidth, uint32_t unHeight, uint32_t unDepth) {
	STUBBED();
}
EVROverlayError BaseOverlay::SetOverlayFromFile(VROverlayHandle_t ulOverlayHandle, const char *pchFilePath) {
	STUBBED();
}
EVROverlayError BaseOverlay::GetOverlayTexture(VROverlayHandle_t ulOverlayHandle, void **pNativeTextureHandle, void *pNativeTextureRef, uint32_t *pWidth, uint32_t *pHeight, uint32_t *pNativeFormat, ETextureType *pAPIType, EColorSpace *pColorSpace, VRTextureBounds_t *pTextureBounds) {
	STUBBED();
}
EVROverlayError BaseOverlay::ReleaseNativeOverlayHandle(VROverlayHandle_t ulOverlayHandle, void *pNativeTextureHandle) {
	STUBBED();
}
EVROverlayError BaseOverlay::GetOverlayTextureSize(VROverlayHandle_t ulOverlayHandle, uint32_t *pWidth, uint32_t *pHeight) {
	STUBBED();
}
EVROverlayError BaseOverlay::CreateDashboardOverlay(const char *pchOverlayKey, const char *pchOverlayFriendlyName, VROverlayHandle_t * pMainHandle, VROverlayHandle_t *pThumbnailHandle) {
	STUBBED();
}
bool BaseOverlay::IsDashboardVisible() {
	// TODO should this be based of whether Dash is open?
	// Probably, but handling focus opens some other issues as it triggers under other conditions.
	return false;
}
bool BaseOverlay::IsActiveDashboardOverlay(VROverlayHandle_t ulOverlayHandle) {
	STUBBED();
}
EVROverlayError BaseOverlay::SetDashboardOverlaySceneProcess(VROverlayHandle_t ulOverlayHandle, uint32_t unProcessId) {
	STUBBED();
}
EVROverlayError BaseOverlay::GetDashboardOverlaySceneProcess(VROverlayHandle_t ulOverlayHandle, uint32_t *punProcessId) {
	STUBBED();
}
void BaseOverlay::ShowDashboard(const char *pchOverlayToShow) {
	STUBBED();
}
TrackedDeviceIndex_t BaseOverlay::GetPrimaryDashboardDevice() {
	STUBBED();
}
EVROverlayError BaseOverlay::ShowKeyboardWithDispatch(EGamepadTextInputMode eInputMode, EGamepadTextInputLineMode eLineInputMode,
	const char * pchDescription, uint32_t unCharMax, const char * pchExistingText, bool bUseMinimalMode, uint64_t uUserValue,
	VRKeyboard::eventDispatch_t eventDispatch) {

#ifndef OC_XR_PORT
	if (!BaseCompositor::dxcomp)
		OOVR_ABORT("Keyboard currently only available on DX11 and DX10 games");

	if (eLineInputMode != k_EGamepadTextInputLineModeSingleLine)
		OOVR_ABORTF("Only single-line keyboard entry mode is currently supported (as opposed to ID=%d)", eLineInputMode);

	// TODO use description
	keyboard = make_unique<VRKeyboard>(BaseCompositor::dxcomp->GetDevice(), uUserValue, unCharMax, bUseMinimalMode, eventDispatch,
		(VRKeyboard::EGamepadTextInputMode)eInputMode);

	keyboard->contents(VRKeyboard::CHAR_CONV.from_bytes(pchExistingText));

	BaseSystem *system = GetUnsafeBaseSystem();
	if (system) {
		system->_BlockInputsUntilReleased();
	}
#else
	STUBBED();
#endif

	return VROverlayError_None;
}
EVROverlayError BaseOverlay::ShowKeyboard(EGamepadTextInputMode eInputMode, EGamepadTextInputLineMode eLineInputMode,
	const char *pchDescription, uint32_t unCharMax, const char *pchExistingText, bool bUseMinimalMode, uint64_t uUserValue) {

	VRKeyboard::eventDispatch_t dispatch = [](VREvent_t ev) {
		BaseSystem *sys = GetUnsafeBaseSystem();
		if (sys) {
			sys->_EnqueueEvent(ev);
		}
	};

	return ShowKeyboardWithDispatch(eInputMode, eLineInputMode, pchDescription, unCharMax, pchExistingText, bUseMinimalMode, uUserValue, dispatch);
}
EVROverlayError BaseOverlay::ShowKeyboard(EGamepadTextInputMode eInputMode, EGamepadTextInputLineMode eLineInputMode, uint32_t unFlags,
		const char *pchDescription, uint32_t unCharMax, const char *pchExistingText, uint64_t uUserValue) {
	STUBBED();
}
EVROverlayError BaseOverlay::ShowKeyboardForOverlay(VROverlayHandle_t ulOverlayHandle,
	EGamepadTextInputMode eInputMode, EGamepadTextInputLineMode eLineInputMode,
	const char *pchDescription, uint32_t unCharMax, const char *pchExistingText,
	bool bUseMinimalMode, uint64_t uUserValue) {

	USEH();

	VRKeyboard::eventDispatch_t dispatch = [overlay](VREvent_t ev) {
		overlay->eventQueue.push(ev);
	};

	return ShowKeyboardWithDispatch(eInputMode, eLineInputMode, pchDescription, unCharMax, pchExistingText, bUseMinimalMode, uUserValue, dispatch);
}
EVROverlayError BaseOverlay::ShowKeyboardForOverlay(VROverlayHandle_t ulOverlayHandle, EGamepadTextInputMode eInputMode,
		EGamepadTextInputLineMode eLineInputMode, uint32_t unFlags, const char *pchDescription, uint32_t unCharMax,
		const char *pchExistingText, uint64_t uUserValue) {
	STUBBED();
}
uint32_t BaseOverlay::GetKeyboardText(char *pchText, uint32_t cchText) {
	string str = keyboard ? VRKeyboard::CHAR_CONV.to_bytes(keyboard->contents()) : keyboardCache;

	// FFS, strncpy is secure.
	strncpy_s(pchText, cchText, str.c_str(), cchText);

	// Ensure the array always ends in a NULL
	pchText[cchText - 1] = 0;

	// TODO is this supposed to return the length of the string including or excluding the cchText limit?
	return (uint32_t)strlen(pchText);
}
void BaseOverlay::HideKeyboard() {
	// First, if the keyboard is currently open, cache it's contents
	if (keyboard) {
		keyboardCache = VRKeyboard::CHAR_CONV.to_bytes(keyboard->contents());
	}

	// Delete the keyboard instance
	keyboard.reset();

	BaseSystem *system = GetUnsafeBaseSystem();
	if (system) {
		system->_BlockInputsUntilReleased();
	}
}
void BaseOverlay::SetKeyboardTransformAbsolute(ETrackingUniverseOrigin eTrackingOrigin, const HmdMatrix34_t *pmatTrackingOriginToKeyboardTransform) {
	if (!keyboard)
		OOVR_ABORT("Cannot set keyboard position when the keyboard is closed!");

#ifdef OC_XR_PORT
	XR_STUBBED();
#else
	BaseCompositor *compositor = GetUnsafeBaseCompositor();
	if (compositor && eTrackingOrigin != compositor->GetTrackingSpace()) {
		OOVR_ABORTF("Origin mismatch - current %d, requested %d", compositor->GetTrackingSpace(), eTrackingOrigin);
	}

	keyboard->SetTransform(*pmatTrackingOriginToKeyboardTransform);
#endif
}
void BaseOverlay::SetKeyboardPositionForOverlay(VROverlayHandle_t ulOverlayHandle, HmdRect2_t avoidRect) {
	STUBBED();
}
EVROverlayError BaseOverlay::SetOverlayIntersectionMask(VROverlayHandle_t ulOverlayHandle, OOVR_VROverlayIntersectionMaskPrimitive_t *pMaskPrimitives, uint32_t unNumMaskPrimitives, uint32_t unPrimitiveSize) {
	STUBBED();
}
EVROverlayError BaseOverlay::GetOverlayFlags(VROverlayHandle_t ulOverlayHandle, uint32_t *pFlags) {
	STUBBED();
}
BaseOverlay::VRMessageOverlayResponse BaseOverlay::ShowMessageOverlay(const char* pchText, const char* pchCaption, const char* pchButton0Text, const char* pchButton1Text, const char* pchButton2Text, const char* pchButton3Text) {
	STUBBED();
}
void BaseOverlay::CloseMessageOverlay() {
	STUBBED();
}
