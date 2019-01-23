#include "DrvOculusCommon.h"
#include "pub/drvoculus.h"
#include "OculusBackend.h"

IBackend * DrvOculus::CreateOculusBackend() {
	return new OculusBackend();
}
