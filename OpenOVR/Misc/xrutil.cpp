#include "stdafx.h"

#include "xrutil.h"

#include "xr_ext.h"

XrInstance xr_instance = nullptr;
XrSession xr_session = nullptr;
XrSessionState xr_session_state = XR_SESSION_STATE_UNKNOWN;
XrSystemId xr_system = XR_NULL_SYSTEM_ID;
XrViewConfigurationView xr_main_view = {};

XrExt* xr_ext = nullptr;
