#include "stdafx.h"
#define BASE_IMPL
#include "BaseControlPanel.h"

// These functions are used in HL:A - See #176
// Nooping both of these should be fine.
void BaseControlPanel::SetDashBoardUserToggleEnabled(bool status) {
}
OOVR_EVR_ControlPanel_Error BaseControlPanel::RegisterExternalWebRoot() {
	return VR_ControlPanelError_None;
}

// Unknown functions
#define UKN_FUNC(id) void BaseControlPanel::UknFunc ## id () { \
STUBBED(); \
}

UKN_FUNC(00);
UKN_FUNC(01);
UKN_FUNC(02);
UKN_FUNC(03);
UKN_FUNC(04);
UKN_FUNC(05);
UKN_FUNC(06);
UKN_FUNC(07);
UKN_FUNC(08);
UKN_FUNC(09);
UKN_FUNC(10);
UKN_FUNC(11);
UKN_FUNC(12);
UKN_FUNC(13);
UKN_FUNC(14);
UKN_FUNC(16);
UKN_FUNC(17);
UKN_FUNC(18);
UKN_FUNC(19);
UKN_FUNC(20);
UKN_FUNC(21);
UKN_FUNC(22);
UKN_FUNC(23);
UKN_FUNC(24);
UKN_FUNC(25);
UKN_FUNC(27);
UKN_FUNC(28);
UKN_FUNC(29);
