#pragma once

#include "../BaseCommon.h"
#include "custom_interfaces/IVRMailbox_001.h"

enum OOVR_EVR_ControlPanel_Error {
	VR_ControlPanelError_None = 0,
};

class BaseControlPanel {
public:
	void UknFunc00();
	void UknFunc01();
	void UknFunc02();
	void UknFunc03();
	void UknFunc04();
	void UknFunc05();
	void UknFunc06();
	void UknFunc07();
	void UknFunc08();
	void UknFunc09();
	void UknFunc10();
	void UknFunc11();
	void UknFunc12();
	void UknFunc13();
	void UknFunc14();
	void SetDashBoardUserToggleEnabled(bool status);
	void UknFunc16();
	void UknFunc17();
	void UknFunc18();
	void UknFunc19();
	void UknFunc20();
	void UknFunc21();
	void UknFunc22();
	void UknFunc23();
	void UknFunc24();
	void UknFunc25();
	OOVR_EVR_ControlPanel_Error RegisterExternalWebRoot();
	void UknFunc27();
	void UknFunc28();
	void UknFunc29();
};
