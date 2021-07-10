#pragma once
#include "../interfaces/openvr.h"
#include "../interfaces/vrtypes.h"
#include "../interfaces/vrannotation.h"

// Used by Half-Life: Alyx. Completely propriatary and undocumented interface.
// And this, by the way, is why I'm not particularly fond of working on Valve games.

namespace vr
{
namespace IVRInputInternal_002
{

/** Allows the application to interact with the compositor */
class IVRInputInternal
{
public:
	// Hopefully they have <20 functions...
	virtual void UknFunc001() = 0;
	virtual void UknFunc002() = 0;
	virtual void UknFunc003() = 0;
	virtual void UknFunc004() = 0;
	virtual void UknFunc005() = 0;
	virtual void UknFunc006() = 0;
	virtual void UknFunc007() = 0;
	virtual void UknFunc008() = 0;
	virtual void UknFunc009() = 0;
	virtual void UknFunc010() = 0;
	virtual void UknFunc011() = 0;
	virtual void UknFunc012() = 0;
	virtual void UknFunc013() = 0;
	virtual void UknFunc014() = 0;
	virtual void UknFunc015() = 0;
	virtual void UknFunc016() = 0;
	virtual void UknFunc017() = 0;
	virtual void UknFunc018() = 0;
	virtual void UknFunc019() = 0;
	virtual void UknFunc020() = 0;
};

static const char * const IVRInputInternal_Version = "IVRInputInternal_002";

} // namespace vr



} // Close custom namespace
