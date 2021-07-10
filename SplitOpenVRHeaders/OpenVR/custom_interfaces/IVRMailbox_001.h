#pragma once
#include "../interfaces/openvr.h"
#include "../interfaces/vrtypes.h"
#include "../interfaces/vrannotation.h"

// Used by Half-Life: Alyx. Completely propriatary and undocumented interface.
// Taken from:
// https://github.com/ValveSoftware/Proton/blob/22f40122788d5f65389f59144705309ce9d6d403/vrclient_x64/openvr_v1.10.30/openvr.h
// ... which has all the types 'removed' as such. So much for 'Open'VR.

namespace vr
{
namespace IVRMailbox_001
{

typedef uint64_t mbox_handle;

enum vrmb_typeb
{
	valuea = 0,
};

class IVRMailbox
{
public:
	virtual vrmb_typeb RegisterMailbox(const char *name, mbox_handle *handle) = 0;
	virtual vrmb_typeb undoc2(mbox_handle handle) = 0;
	virtual vrmb_typeb undoc3(mbox_handle handle, const char *b, const char *c) = 0;
	virtual vrmb_typeb ReadMessage(mbox_handle mbox, char* outBuf, uint32_t outBufLen, uint32_t* msgLen) = 0;
};

static const char * const IVRMailbox_Version = "IVRMailbox_001";

} // namespace vr

} // Close custom namespace
