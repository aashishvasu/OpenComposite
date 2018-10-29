#include "stdafx.h"
#define GENFILE
#include "BaseCommon.h"

GEN_INTERFACE("Compositor", "012")
GEN_INTERFACE("Compositor", "013")
GEN_INTERFACE("Compositor", "014")
GEN_INTERFACE("Compositor", "015")
GEN_INTERFACE("Compositor", "016")
GEN_INTERFACE("Compositor", "017", CUSTOM)
GEN_INTERFACE("Compositor", "018")
GEN_INTERFACE("Compositor", "019")
GEN_INTERFACE("Compositor", "020")
GEN_INTERFACE("Compositor", "021")
GEN_INTERFACE("Compositor", "022")

#include "GVRCompositor.gen.h"

using namespace vr;

// Screenshot functions, only in version 015, and marked as deprecated at that. It's highly
//  unlikely these will be used by any publicly-available application.
// Probably added by Valve for internal usage, and deprecated before it was released.
IVRCompositor_015::EVRCompositorError CVRCompositor_015::RequestScreenshot(EVRScreenshotType type,
	const char *pchDestinationFileName, const char *pchVRDestinationFileName) {

	STUBBED();
}

EVRScreenshotType CVRCompositor_015::GetCurrentScreenshotType() {
	STUBBED();
}
