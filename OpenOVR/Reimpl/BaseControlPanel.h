#pragma once

#include "../BaseCommon.h"
#include "OpenVR/custom_interfaces/IVRMailbox_001.h"

enum OOVR_EVR_ControlPanel_Error {
	VR_ControlPanelError_None = 0,
};

class BaseControlPanel {
public:
	virtual void UknFunc00();
	virtual void UknFunc01();
	virtual void UknFunc02();
	virtual void UknFunc03();
	virtual void UknFunc04();
	virtual void UknFunc05();
	virtual void UknFunc06();
	virtual void UknFunc07();
	virtual void UknFunc08();
	virtual void UknFunc09();
	virtual void UknFunc10();
	virtual void UknFunc11();
	virtual void UknFunc12();
	virtual void UknFunc13();
	virtual void UknFunc14();
	virtual void SetDashBoardUserToggleEnabled(bool status);
	virtual void UknFunc16();
	virtual void UknFunc17();
	virtual void UknFunc18();
	virtual void UknFunc19();
	virtual void UknFunc20();
	virtual void UknFunc21();
	virtual void UknFunc22();
	virtual void UknFunc23();
	virtual void UknFunc24();
	virtual void UknFunc25();
	virtual OOVR_EVR_ControlPanel_Error RegisterExternalWebRoot();
	virtual void UknFunc27();
	virtual void UknFunc28();
	virtual void UknFunc29();
};
