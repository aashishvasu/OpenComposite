#pragma once

#include "../BaseCommon.h"
#include "custom_interfaces/IVRMailbox_001.h"

typedef vr::IVRMailbox_001::mbox_handle OOVR_mbox_handle;

class BaseMailbox {
public:
	enum MboxErr {
		VR_MBox_None = 0,
		VR_MBox_NoMessage = 1,
		VR_MBox_BufferTooShort = 2,
	};

	// TODO build up names and comments for these and their types

	MboxErr RegisterMailbox(const char* name, OOVR_mbox_handle* handle);

	MboxErr UnregisterMailbox(OOVR_mbox_handle mbox);

	MboxErr SendMessage(OOVR_mbox_handle mbox, const char* type, const char* message);

	MboxErr ReadMessage(OOVR_mbox_handle mbox, char* outBuf, uint32_t outBufLen, uint32_t* msgLen);
};

typedef BaseMailbox::MboxErr OOVR_vrmb_typeb;
