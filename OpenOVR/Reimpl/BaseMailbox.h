#pragma once

#include "../BaseCommon.h"
#include "OpenVR/custom_interfaces/IVRMailbox_001.h"

typedef vr::IVRMailbox_001::vrmb_typea OOVR_vrmb_typea;
typedef vr::IVRMailbox_001::vrmb_typeb OOVR_vrmb_typeb;

class BaseMailbox
{
public:
	// TODO build up names and comments for these and their types

	OOVR_vrmb_typeb undoc1(const char *a, OOVR_vrmb_typea *b);

	OOVR_vrmb_typeb undoc2(OOVR_vrmb_typea a);

	OOVR_vrmb_typeb undoc3(OOVR_vrmb_typea a, const char *b, const char *c);

	OOVR_vrmb_typeb undoc4(OOVR_vrmb_typea a, char *b, uint32_t c, uint32_t *d);
};
