#pragma once
#include "../interfaces/openvr.h"
#include "../interfaces/vrtypes.h"
#include "../interfaces/vrannotation.h"

// Used by Half-Life: Alyx. Completely propriatary and undocumented interface.

namespace vr
{
namespace IVRControlPanel_006
{

typedef int EVR_ControlPanel_Error;

class IVRControlPanel
{
public:
	virtual void UknFunc00() = 0;
	virtual void UknFunc01() = 0;
	virtual void UknFunc02() = 0;
	virtual void UknFunc03() = 0;
	virtual void UknFunc04() = 0;
	virtual void UknFunc05() = 0;
	virtual void UknFunc06() = 0;
	virtual void UknFunc07() = 0;
	virtual void UknFunc08() = 0;
	virtual void UknFunc09() = 0;
	virtual void UknFunc10() = 0;
	virtual void UknFunc11() = 0;
	virtual void UknFunc12() = 0;
	virtual void UknFunc13() = 0;
	virtual void UknFunc14() = 0;
	virtual void SetDashBoardUserToggleEnabled(bool status) = 0;
	virtual void UknFunc16() = 0;
	virtual void UknFunc17() = 0;
	virtual void UknFunc18() = 0;
	virtual void UknFunc19() = 0;
	virtual void UknFunc20() = 0;
	virtual void UknFunc21() = 0;
	virtual void UknFunc22() = 0;
	virtual void UknFunc23() = 0;
	virtual void UknFunc24() = 0;
	virtual void UknFunc25() = 0;
	virtual EVR_ControlPanel_Error RegisterExternalWebRoot() = 0;
	virtual void UknFunc27() = 0;
	virtual void UknFunc28() = 0;
	virtual void UknFunc29() = 0;
};

static const char * const IVRControlPanel_Version = "IVRControlPanel_006";

} // namespace vr

} // Close custom namespace
