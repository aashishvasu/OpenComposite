#pragma once

#include "../BaseCommon.h"
#include "OpenVR/custom_interfaces/IVRMailbox_001.h"

typedef vr::IVRMailbox_001::mbox_handle OOVR_mbox_handle;

class BaseMailbox {
public:
	enum MboxErr {
		VR_MBox_None = 0,
	};

	// TODO build up names and comments for these and their types

	MboxErr RegisterMailbox(const char* name, OOVR_mbox_handle* handle);

	MboxErr undoc2(OOVR_mbox_handle a);

	MboxErr undoc3(OOVR_mbox_handle a, const char* b, const char* c);

	MboxErr ReadMessage(OOVR_mbox_handle mbox, char* outBuf, uint32_t outBufLen, uint32_t* msgLen);
};

typedef BaseMailbox::MboxErr OOVR_vrmb_typeb;
