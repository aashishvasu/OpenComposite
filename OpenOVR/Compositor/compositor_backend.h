#pragma once

#include "compositor.h"

// Include compositor backends as indicated by compiler flags

#ifdef SUPPORT_DX
	// Let's really hope noone tries to use DX10
	#ifdef SUPPORT_DX10
		#include "dx10compositor.h"
	#endif
	#ifdef SUPPORT_DX11
		#include "dx11compositor.h"
	#endif
	#ifdef SUPPORT_DX12
			#include "dx12compositor.h"
	#endif
#endif
#ifdef SUPPORT_VK
#include "vkcompositor.h"
#endif
#ifdef SUPPORT_GL
#include "glcompositor.h"
#endif
