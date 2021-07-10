/**
 * Derived from audio-router (https://github.com/audiorouterdev/audio-router), GPLv3
 *
 * This hooks the 'Activate' method on the default audio device, and makes
 * it activate the Rift's audio device instead.
 */

#include "stdafx.h"
#include "audio_override.h"

#include <Windows.h>
#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <string>
#include <cassert>
#include <atlbase.h>
#include <Mmsystem.h>

#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "winmm.lib")
#include <type_traits>
#include <stdint.h>

// TODO: atomicity mus be implemented with short jmp to code path in 2gb address space
// that will jump to the patched function in the 64 space

template<typename T>
class patcher {
public:
	typedef T func_t;
	typedef size_t address_t;
#pragma pack(push, 1)
	struct jmp_to {
		typedef typename std::conditional<
			std::is_same<address_t, uint32_t>::value,
			std::integral_constant<unsigned char, 0xb8>,
			std::integral_constant<WORD, 0xb848>>::type mov_ax_t;
		typename mov_ax_t::value_type mov_ax = mov_ax_t::value;
		address_t addr;
		WORD jmp_ax = 0xe0ff;
	};
#pragma pack(pop)
private:
	const func_t patched_func;
	void* original_func;
	jmp_to old_bytes;
	DWORD old_protect;
	CRITICAL_SECTION critical_section;
public:
	patcher(func_t patched_func) : patched_func(patched_func), original_func(NULL) {
		InitializeCriticalSectionAndSpinCount(&this->critical_section, 0x00000400);
	}

	~patcher() {
		this->destroy();
	}

	void destroy() {
		this->revert();
		DeleteCriticalSection(&this->critical_section);
	}

	int is_patched() const {
		if (IsBadReadPtr(this->original_func, sizeof(jmp_to)))
			return 2;

		return (int)(memcmp(this->original_func, &this->old_bytes, sizeof(jmp_to)) != 0);
	}

	const void* get_function() const { return this->original_func; }

	int patch(void* func_address) {
		if (!func_address)
			return 1;

		//// patchable function must be 16 byte aligned to ensure atomic patching
		//if((address_t)func_address & 0xf)
		//    return 3;

		//#ifdef _WIN64
		//        const size_t size = 16;
		//#else
		//        const size_t size = 8;
		//#endif
		//        assert(size >= sizeof(jmp_to));
		//
		if (!VirtualProtect(func_address, sizeof(jmp_to), PAGE_EXECUTE_READWRITE, &this->old_protect))
			return 2;

		this->original_func = func_address;
		memcpy(&this->old_bytes, this->original_func, sizeof(jmp_to));
		this->apply();

		return 0;
	}

	void lock() { EnterCriticalSection(&this->critical_section); }
	void unlock() { LeaveCriticalSection(&this->critical_section); }

	void revert() {
		if (IsBadWritePtr(this->original_func, sizeof(jmp_to)))
			return;

		//if(this->patched)
		{
			// bad write ptr might happen if the dll that is patched
			// is unloaded before this dll is unloaded
			/*if(IsBadWritePtr(this->original_func, sizeof(jmp_to)))
			return;*/
			memcpy(this->original_func, &this->old_bytes, sizeof(jmp_to));
			//this->patched = false;
		}
	}

	void apply() {
		if (IsBadWritePtr(this->original_func, sizeof(jmp_to)))
			return;

		//if(!this->patched)
		{
			jmp_to patch;
			patch.addr = (address_t)this->patched_func;
			memcpy(this->original_func, &patch, sizeof(jmp_to));
			//this->patched = true;
		}
	}
};

template<typename T>
struct scope_patcher_disable {
	T &pat;
	scope_patcher_disable(T &pat) : pat(pat) {
		pat.revert();
	}
	~scope_patcher_disable() {
		pat.apply();
	}
};

struct crit_section_lock {
	CRITICAL_SECTION &crit;
	crit_section_lock(CRITICAL_SECTION &crit) : crit(crit) {
		EnterCriticalSection(&crit);
	}
	~crit_section_lock() {
		LeaveCriticalSection(&crit);
	}
};

typedef HRESULT(__stdcall *activate_t)(IMMDevice*, REFIID, DWORD, PROPVARIANT*, void**);
HRESULT __stdcall activate_patch(IMMDevice*, REFIID, DWORD, PROPVARIANT*, void**);

patcher<activate_t> patch_activate(activate_patch);
CRITICAL_SECTION CriticalSection;
std::wstring target_device_id;

static void ensure_com_open() {
	static bool done = false;
	if (done)
		return;
	else
		done = true;

	// Don't care about the result
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
}

HRESULT find_basic_rift_output_device(std::wstring &output) {
	HRESULT hr;
	CComPtr<IMMDeviceEnumerator> groupEnum;
	CComPtr<IMMDeviceCollection> group;

	// If we don't find anything, make sure we return the string empty
	output.empty();

	ensure_com_open();

	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&groupEnum));
	if (hr != S_OK)
		return hr;

	hr = groupEnum->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &group);
	if (hr != S_OK)
		return hr;

	UINT count;
	hr = group->GetCount(&count);
	if (hr)
		return hr;

	for (ULONG i = 0; i < count; i++) {
		CComPtr<IMMDevice> dev;
		group->Item(i, &dev);

		CComPtr<IPropertyStore> props;
		hr = dev->OpenPropertyStore(
			STGM_READ, &props);
		if (hr)
			return hr;

		PROPVARIANT varName;
		// Initialize container for property value.
		PropVariantInit(&varName);

		hr = props->GetValue(PKEY_Device_FriendlyName, &varName);
		if (hr)
			return hr;

		std::wstring friendlyName = varName.pwszVal;
		PropVariantClear(&varName);

		for (std::wstring::iterator it = friendlyName.begin(); it != friendlyName.end(); ++it)
			*it = towlower(*it);

		// IDK if the friendly name is localised, so search for just 'rift'
		//  which should hopefully work
		if (friendlyName.find(L"rift") == std::string::npos) {
			continue;
		}

		LPWSTR dev_id = NULL;
		hr = dev->GetId(&dev_id);
		if (hr)
			continue;

		output = dev_id;

		CoTaskMemFree(dev_id);

		// In case something else contains 'rift', prefer 'rift audio'
		if (friendlyName.find(L"rift audio") != std::string::npos) {
			return 0;
		}
	}

	return 0;
}

void set_app_default_audio_device(std::wstring device_id) {
	target_device_id = device_id;

	ensure_com_open();

	static bool done = false;
	if (done)
		return;
	done = true;

	CComPtr<IMMDeviceEnumerator> pEnumerator;
	CComPtr<IMMDevice> pDevice;

	HRESULT hr = CoCreateInstance(
		__uuidof(MMDeviceEnumerator), NULL,
		CLSCTX_ALL, IID_PPV_ARGS(&pEnumerator));
	if (hr != S_OK)
		return;

	hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
	if (hr != S_OK)
		return;

	IMMDevice *dev = pDevice;
	DWORD* patch_activate_ptr = ((DWORD***)dev)[0][3];

	InitializeCriticalSectionAndSpinCount(&CriticalSection, 0x00000400);
	patch_activate.patch(patch_activate_ptr);

	// critical section necessary in case if another thread is currently
	// performing initialization
	{
		crit_section_lock section_lock(CriticalSection);
		patch_activate.apply();
	}
}

static HRESULT is_device_in_group(IMMDevice *dev, IMMDeviceCollection *group, bool *found) {
	*found = false;

	UINT count;
	HRESULT hr = group->GetCount(&count);
	if (hr)
		return hr;

	LPWSTR dev_id = NULL;
	hr = dev->GetId(&dev_id);
	if (hr)
		return hr;

	if (!dev_id)
		return 0;

	for (ULONG i = 0; i < count; i++) {
		CComPtr<IMMDevice> pEndpoint;
		LPWSTR pwszID;

		group->Item(i, &pEndpoint);
		// Get the endpoint ID string.
		pEndpoint->GetId(&pwszID);
		// check if the this_ endpoint device is in render group
		if (!*found && wcscmp(pwszID, dev_id) == 0)
			*found = true;

		CoTaskMemFree(pwszID);
	}
	CoTaskMemFree(dev_id);

	return 0;
}

static HRESULT __stdcall activate_patch(
	IMMDevice* this_, REFIID iid, DWORD dwClsCtx,
	PROPVARIANT* pActivationParams, void** ppInterface) {

	crit_section_lock section_lock(CriticalSection);
	scope_patcher_disable<patcher<activate_t>> _disable(patch_activate);

	// use default since default audio device has been requested
	if (target_device_id.empty()) {
		return this_->Activate(iid, dwClsCtx, pActivationParams, ppInterface);
	}

	HRESULT hr;
	CComPtr<IMMDeviceEnumerator> groupEnum;
	CComPtr<IMMDeviceCollection> group;

	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&groupEnum));
	if (hr != S_OK)
		return hr;

	hr = groupEnum->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &group);
	if (hr != S_OK)
		return hr;

	// Check the device we're modifying is from the same group as the device we want to replace it with.
	// We wouldn't want to replace a microphone with a speaker or vice-versa.
	bool endpoint_found;
	hr = is_device_in_group(this_, group, &endpoint_found);
	if (hr != S_OK)
		return hr;

	if (!endpoint_found) {
		return this_->Activate(iid, dwClsCtx, pActivationParams, ppInterface);
	}

	// Grab the target device
	CComPtr<IMMDevice> targetDevice;
	hr = groupEnum->GetDevice(target_device_id.c_str(), &targetDevice);
	if (hr != S_OK)
		return hr;

	if (!targetDevice)
		return AUDCLNT_E_DEVICE_INVALIDATED;

	// And activate it
	return targetDevice->Activate(iid, dwClsCtx, pActivationParams, ppInterface);
}
