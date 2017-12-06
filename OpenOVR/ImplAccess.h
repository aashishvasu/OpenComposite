#pragma once

#include "Reimpl/CVRSystem.h"
#include "Reimpl/CVRRenderModels.h"
#include "Reimpl/CVRCompositor.h"

struct CVRImplAccess_ {
	CVRSystem *vr_System;
	CVRRenderModels *vr_RenderModels;
	CVRCompositor *vr_Compositor;
};

extern CVRImplAccess_ CVRImplAccess;
