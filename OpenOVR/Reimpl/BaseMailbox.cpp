#include "stdafx.h"

#define BASE_IMPL

#include "BaseMailbox.h"

typedef BaseMailbox::MboxErr MboxErr;

MboxErr BaseMailbox::RegisterMailbox(const char* name, OOVR_mbox_handle* handle)
{
	*handle = 123;

	// Ignore it for now
	OOVR_LOGF("Pretending to register mailbox '%s'", name);

	return VR_MBox_None;
}

MboxErr BaseMailbox::undoc2(OOVR_mbox_handle a)
{
	STUBBED();
}

MboxErr BaseMailbox::undoc3(OOVR_mbox_handle a, const char* b, const char* c)
{
	STUBBED();
}

MboxErr BaseMailbox::ReadMessage(OOVR_mbox_handle mboxHandle, char* outBuf, uint32_t outBufLen, uint32_t* msgLen)
{
	// Read message function
	// If we have a message available (IDK what the mailbox is even supposed to do), try to read a message.
	// If the message fits within the output buffer, copy it in and return 0.
	// Otherwise if it doesn't fit, return 2.
	// In both cases, set msgLen
	// If we don't have a message, return an unknown error code other than 0 and 2 (HL:A doesn't care).

	// Fow now, just say we don't have a message
	return (MboxErr)12345;
}
