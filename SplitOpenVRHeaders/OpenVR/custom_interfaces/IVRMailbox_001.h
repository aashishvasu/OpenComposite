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

typedef uint64_t vrmb_typea;

enum vrmb_typeb
{
	valuea = 0,
};

class IVRMailbox
{
public:
	virtual vrmb_typeb undoc1( const char *a, vrmb_typea *b ) = 0;
	virtual vrmb_typeb undoc2( vrmb_typea a ) = 0;
	virtual vrmb_typeb undoc3( vrmb_typea a, const char *b, const char *c ) = 0;
	virtual vrmb_typeb undoc4( vrmb_typea a, char *b, uint32_t c, uint32_t *d ) = 0;
};

static const char * const IVRMailbox_Version = "IVRMailbox_001";

} // namespace vr

} // Close custom namespace
