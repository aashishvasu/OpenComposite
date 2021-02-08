#include "stdafx.h"

#include "VRKeyboard.h"

#ifndef OC_XR_PORT
#include <d3d11.h>
#endif

#include "Reimpl/static_bases.gen.h"
#include "Reimpl/BaseCompositor.h"
#include "Reimpl/BaseSystem.h"

#include "Misc/ScopeGuard.h"
#include "convert.h"

#include "resources.h"

#include <vector>

#ifdef _WIN32
#pragma comment(lib, "d3d11.lib")

// for debugging only for now
#include <comdef.h>
#endif

using namespace std;

// If you're working on the keyboard, it may be useful to have it headlocked so you can easily see it in the Oculus mirror
static const bool DBG_STUCK_TO_FACE = false;

static vector<char> loadResource(int rid, int type) {
#ifdef _WIN32
	// Open our OBJ file
	HRSRC ref = FindResource(openovr_module_id, MAKEINTRESOURCE(rid), MAKEINTRESOURCE(type));
	if (!ref) {
		string err = "FindResource error: " + std::to_string(GetLastError());
		OOVR_ABORT(err.c_str());
	}

	char *cstr = (char*)LoadResource(openovr_module_id, ref);
	if (!cstr) {
		string err = "LoadResource error: " + std::to_string(GetLastError());
		OOVR_ABORT(err.c_str());
	}

	DWORD len = SizeofResource(openovr_module_id, ref);
	if (!len) {
		string err = "SizeofResource error: " + std::to_string(GetLastError());
		OOVR_ABORT(err.c_str());
	}

	// Do we need to use UnlockResource on cstr?

	return vector<char>(cstr, cstr + len);
#else
#ifdef OC_XR_PORT
	XR_STUBBED();
#else
#error TODO implement keyboard font loading on Linux
#endif
#endif
}

std::wstring_convert<std::codecvt_utf8<wchar_t>> VRKeyboard::CHAR_CONV;

#ifndef OC_XR_PORT

VRKeyboard::VRKeyboard(ID3D11Device *dev, uint64_t userValue, uint32_t maxLength, bool minimal, eventDispatch_t eventDispatch,
	EGamepadTextInputMode inputMode)
	: dev(dev), userValue(userValue), maxLength(maxLength), minimal(minimal), eventDispatch(eventDispatch), inputMode(inputMode) {

	std::shared_ptr<BaseCompositor> cmp = GetBaseCompositor();
	if (!cmp)
		OOVR_ABORT("Keyboard: Compositor must be active!");

	if(!dev)
		OOVR_ABORT("Keyboard currently only works on DX11, and game must have submitted at least a single frame");

	if (inputMode == EGamepadTextInputMode::k_EGamepadTextInputModePassword)
		OOVR_ABORT("Password input mode not yet supported!");

	// zero stuff out
	memset(lastInputTime, 0, sizeof(lastInputTime));
	memset(repeatCount, 0, sizeof(repeatCount));
	memset(selected, 0, sizeof(selected));
	memset(lastButtonState, 0, sizeof(lastButtonState));

	// D3D setup
	dev->GetImmediateContext(&ctx);

	Sizei bufferSize(1024, 512);

	ovrTextureSwapChainDesc &desc = chainDesc;
	desc = {};
	desc.Type = ovrTexture_2D;
	desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.ArraySize = 1;
	desc.Width = bufferSize.w;
	desc.Height = bufferSize.h;
	desc.MipLevels = 1;
	desc.SampleCount = 1;
	desc.StaticImage = ovrFalse;
	desc.MiscFlags = ovrTextureMisc_None;
	desc.BindFlags = ovrTextureBind_None;

	OOVR_FAILED_OVR_ABORT(ovr_CreateTextureSwapChainDX(*ovr::session, dev, &desc, &chain));

	// Create HUD layer, fixed to the player's torso
	memset(&layer, 0, sizeof(layer));
	layer.Header.Type = ovrLayerType_Quad;
	layer.Header.Flags = DBG_STUCK_TO_FACE ? ovrLayerFlag_HeadLocked : 0; // | ovrLayerFlag_HighQuality;;
	layer.ColorTexture = chain;
	// 50cm in front and 20cm down from the player's nose,
	// fixed relative to their torso.
	layer.QuadPoseCenter.Position.x = 0.00f;
	layer.QuadPoseCenter.Position.y = 0.00f; // -0.20f;
	layer.QuadPoseCenter.Position.z = -0.50f;
	layer.QuadPoseCenter.Orientation.x = 0;
	layer.QuadPoseCenter.Orientation.y = 0;
	layer.QuadPoseCenter.Orientation.z = 0;
	layer.QuadPoseCenter.Orientation.w = 1;
	// HUD is 50cm wide, 30cm tall.
	layer.QuadSize.x = 0.50f;
	layer.QuadSize.y = 0.30f;

	// Display all of the HUD texture.
	layer.Viewport.Pos.x = 0;
	layer.Viewport.Pos.y = 0;
	layer.Viewport.Size.w = desc.Width;
	layer.Viewport.Size.h = desc.Height;

	font = make_unique<SudoFontMeta>(loadResource(RES_O_FNT_UBUNTU, RES_T_FNTMETA), loadResource(RES_O_FNT_UBUNTU, RES_T_PNG));
	layout = make_unique<KeyboardLayout>(loadResource(RES_O_KB_EN_GB, RES_T_KBLAYOUT));

	if (!DBG_STUCK_TO_FACE) {
		BaseCompositor *comp = GetUnsafeBaseCompositor();
		vr::TrackedDevicePose_t spose = { 0 };
		comp->GetLastPoses(&spose, 1, nullptr, 0);

		OVR::Posef offset(OVR::Quatf::Identity(), OVR::Vector3f(0.0f, 0.0f, -1.0f));
		OVR::Posef headPose = S2O_om34_pose(spose.mDeviceToAbsoluteTracking);
		layer.QuadPoseCenter = headPose * offset;
	}
}

VRKeyboard::~VRKeyboard() {
	if (ctx)
		ctx->Release();
}

#else
VRKeyboard::~VRKeyboard()
{
	XR_STUBBED();
}
#endif
wstring VRKeyboard::contents() {
	return text;
}
#ifndef OC_XR_PORT

void VRKeyboard::contents(wstring str) {
	text = str;
	dirty = true;
}

ovrLayerHeader * VRKeyboard::Update() {
	if (dirty) {
		dirty = false;
		Refresh();
	}

	return &layer.Header;
}

void VRKeyboard::HandleOverlayInput(vr::EVREye side, vr::VRControllerState_t state, float time) {
	using namespace vr;

	// In case this is somehow called after the keyboard is closed, ignore it
	if (IsClosed())
		return;

	uint64_t lastButtons = lastButtonState[side];
	lastButtonState[side] = state.ulButtonPressed;

#define GET_BTTN(var, key) bool var = state.ulButtonPressed & ButtonMaskFromId(key)
#define GET_BTTN_LAST(var, key) GET_BTTN(var, key); bool var ## _last = lastButtons & ButtonMaskFromId(key)
	GET_BTTN(left, k_EButton_DPad_Left);
	GET_BTTN(right, k_EButton_DPad_Right);
	GET_BTTN(up, k_EButton_DPad_Up);
	GET_BTTN(down, k_EButton_DPad_Down);
	GET_BTTN_LAST(trigger, k_EButton_SteamVR_Trigger);
	GET_BTTN_LAST(grip, k_EButton_Grip);
#undef GET_BTTN
#undef GET_BTTN_LAST

	if (grip && !grip_last) {
		closed = true;
		SubmitEvent(VREvent_KeyboardClosed, 0);
		return;
	}

	const KeyboardLayout::Key &key = layout->GetKeymap()[selected[side]];

	if (trigger && !trigger_last) {
		wchar_t ch = caseMode == ECaseMode::LOWER ? key.ch : key.shift;

		bool submitKeyEvent = false;

		if (ch == '\x01' || ch == '\x02') {
			// Shift
			ECaseMode target = ch == '\x02' ? ECaseMode::LOCK : ECaseMode::SHIFT;
			caseMode = caseMode == target ? ECaseMode::LOWER : target;
		}
		else if (ch == '\b') {
			// Backspace
			if (!text.empty()) {
				text.erase(text.end() - 1);
			}

			submitKeyEvent = true;
		}
		else if (ch == '\x03') {
			// done

			// Submit mode is for stuff like chat, where the keyboard stays open
			if(inputMode != EGamepadTextInputMode::k_EGamepadTextInputModeSubmit)
				closed = true;

			if(!minimal)
				SubmitEvent(VREvent_KeyboardCharInput, 0);

			SubmitEvent(VREvent_KeyboardDone, 0);
		}
		else if (!minimal && ch == '\t') {
			// Silently soak up tabs for now
		}
		else if (!minimal && ch == '\n') {
			// Silently soak up newlines for now
		}
		else {
			text += ch;

			submitKeyEvent = true;

			if (caseMode == ECaseMode::SHIFT)
				caseMode = ECaseMode::LOWER;
		}

		if (submitKeyEvent) {
			SubmitEvent(VREvent_KeyboardCharInput, minimal ? ch : 0);
		}

		dirty = true;
	}

	// Movement:
	bool any = left || right || up || down;
	if (!any) {
	cancel:
		repeatCount[side] = 0;
		lastInputTime[side] = 0;
		return;
	}

	if (time - lastInputTime[side] < (repeatCount[side] <= 1 ? 0.3 : 0.1))
		return;

	lastInputTime[side] = time;
	repeatCount[side]++;

	int target = -1;
	if (left)
		target = key.toLeft;
	else if (right)
		target = key.toRight;
	else if (up)
		target = key.toUp;
	else if (down)
		target = key.toDown;

	if (target == -1) {
		goto cancel;
	}

	selected[side] = target;
	dirty = true;
}

void VRKeyboard::SetTransform(HmdMatrix34_t transform) {
	ovrPosef pose = S2O_om34_pose(transform);

	layer.QuadPoseCenter = pose;
}

struct pix_t {
	uint8_t r, g, b, a;
};

static_assert(sizeof(pix_t) == 4, "padded pix_t");

void VRKeyboard::Refresh() {
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = chainDesc.Width;
	desc.Height = chainDesc.Height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.SampleDesc = { 1, 0 };
	desc.Usage = D3D11_USAGE_DEFAULT; // D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = 0;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	pix_t *pixels = new pix_t[desc.Width * desc.Height];

	for (UINT y = 0; y < desc.Height; y++) {
		for (UINT x = 0; x < desc.Width; x++) {
			int col = 125;

			pix_t &pix = pixels[x + y * desc.Width];
			pix.a = 255;
			pix.r = col;
			pix.g = col;
			pix.b = col;
		}
	}

	int padding = 8;

	auto fillArea = [pixels, &desc](int x, int y, int w, int h, int r, int g, int b) {
		for (int ix = 0; ix < w; ix++) {
			for (int iy = 0; iy < h; iy++) {
				size_t idx = (x + ix) + (y + iy) * desc.Width;
				pix_t &p = pixels[idx];

				p.r = r;
				p.g = g;
				p.b = b;
				p.a = 255;
			}
		}
	};

	auto print = [&](int x, int y, pix_t colour, wstring text, bool hpad = true) {
		SudoFontMeta::pix_t c = { colour.r, colour.g, colour.b, colour.a };
		for (size_t i = 0; i < text.length(); i++) {
			font->Blit(text[i], x, y, desc.Width, c, (SudoFontMeta::pix_t*)pixels, hpad);
			x += font->Width(text[i]);
		}
	};

	int kbWidth = layout->GetWidth();
	int keySize = ((desc.Width - padding) / kbWidth) - padding;
	auto drawKey = [&](int x, int y, const KeyboardLayout::Key &key) {
		int width = (int)(keySize * key.w);
		int height = (int)(keySize * key.h);

		if (key.spansToRight) {
			width = desc.Width - padding - x;
		}

		bool highlighted = (key.ch == '\x01' && caseMode == ECaseMode::SHIFT)
			|| (key.ch == '\x02' && caseMode == ECaseMode::LOCK);
		int bkg_c = highlighted ? 255 : 80;

		fillArea(
			x, y,
			width, height,
			bkg_c, bkg_c, bkg_c
		);

		if (selected[vr::Eye_Left] == key.id) {
			fillArea(
				x, y,
				width / 2, height,
				0, 100, 255
			);
		}

		if (selected[vr::Eye_Right] == key.id) {
			fillArea(
				x + width / 2, y,
				width / 2, height,
				0, 255, 100
			);
		}

		pix_t targetColour = { 255, 255, 255, 255 };

		if (highlighted) {
			targetColour = { 0, 0, 0, 255 };
		}

		wstring label = caseMode == ECaseMode::LOWER ? key.label : key.labelShift;
		int textWidth = font->Width(label);

		print(x + (width - textWidth) / 2, y + padding, targetColour, label, false);
	};

	int keyAreaBaseY = minimal ? padding : padding + keySize + padding;
	for (const KeyboardLayout::Key &key : layout->GetKeymap()) {
		int x = padding + (int)((keySize + padding) * key.x);
		int y = keyAreaBaseY + (int)((keySize + padding) * key.y);

		drawKey(x, y, key);
	}

	if (!minimal) {
		fillArea(
			padding, padding,
			desc.Width - padding * 2, keySize,
			255, 255, 255
		);

		pix_t targetColour = { 0, 0, 0, 255 };

		print(padding * 2, padding * 2, targetColour, text);
	}

	D3D11_SUBRESOURCE_DATA init[] = {
		{ pixels, sizeof(pix_t) * desc.Width, sizeof(pix_t) * desc.Width * desc.Height }
	};

	CComPtr<ID3D11Texture2D> tex;
	HRESULT rres = dev->CreateTexture2D(&desc, init, &tex);
	//HRESULT hrr = dev->GetDeviceRemovedReason();
	OOVR_FAILED_DX_ABORT(rres);
	delete[] pixels;

	////

	int index;
	OOVR_FAILED_OVR_ABORT(ovr_GetTextureSwapChainCurrentIndex(*ovr::session, chain, &index));

	CComPtr<ID3D11Texture2D> dst;
	OOVR_FAILED_OVR_ABORT(ovr_GetTextureSwapChainBufferDX(*ovr::session, chain, index, IID_PPV_ARGS(&dst)));

	ctx->CopyResource(dst, tex);

	// Commit the texture to LibOVR
	OOVR_FAILED_OVR_ABORT(ovr_CommitTextureSwapChain(*ovr::session, chain));
}

void VRKeyboard::SubmitEvent(vr::EVREventType ev, wchar_t ch) {
	// Here's how (from some basic experimentation) the SteamVR keyboard appears to submit events:
	// In minimal mode:
	// * Pressing a key submits a KeyboardCharInput event, with the character stored in cNewInput
	//    TODO find out how this is encoeded with unicode characters
	// * Clicking of the keyboard submits a KeyboardClosed event, with cNewInput empty (all zeros)
	// * Clicking 'done' submits a KeyboardDone event, with cNewInput empty
	// In standard mode:
	// * cNewInput is always empty
	// * Pressing a key submits a KeyboardCharInput event (the app must read
	//    the text via GetKeyboardText if it wants to know the keyboard contents, since cNewInput is empty)
	// * Clicking of the keyboard submits a KeyboardClosed event
	// * Clicking 'done' submits a KeyboardCharInput event, followed by a KeyboardDone event

	VREvent_Keyboard_t data = { 0 };
	data.uUserValue = userValue;

	memset(data.cNewInput, 0, sizeof(data.cNewInput));

	if (ch != 0) {
		string utf8 = CHAR_CONV.to_bytes(ch);

		if (utf8.length() > sizeof(data.cNewInput)) {
			OOVR_ABORTF("Cannot write symbol '%s' with too many bytes (%d bytes UTF8)", utf8.c_str(), (int)utf8.length());
		}

		memcpy(data.cNewInput, utf8.c_str(), utf8.length());
	}

	VREvent_t evt = { 0 };
	evt.eventType = ev;
	evt.trackedDeviceIndex = 0; // This is accurate to SteamVR
	evt.data.keyboard = data;

	eventDispatch(evt);
}

#endif
