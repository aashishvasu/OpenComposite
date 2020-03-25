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

MboxErr BaseMailbox::undoc4(OOVR_mbox_handle a, char* b, uint32_t c, uint32_t* d)
{
	STUBBED();
}
