#include "DrvOculusCommon.h"
#include "pub/DrvOculus.h"
#include "OculusBackend.h"

IBackend * DrvOculus::CreateOculusBackend() {
	return new OculusBackend();
}
