#include "stdafx.h"
#define GENFILE
#include "BaseCommon.h"

GEN_INTERFACE("Compositor", "019")
GEN_INTERFACE("Compositor", "020")
GEN_INTERFACE("Compositor", "021")
GEN_INTERFACE("Compositor", "022")

#include "GVRCompositor.gen.h"

// TODO Add GetCumulativeStats to the base class
// This didn't happen before due to not knowing how to
// handle structs, but not there isn't any reason why not.
void CVRCompositor_022::GetCumulativeStats(vr::IVRCompositor_022::Compositor_CumulativeStats* pStats, uint32_t nStatsSizeInBytes) {
	STUBBED();
}

void CVRCompositor_021::GetCumulativeStats(vr::IVRCompositor_021::Compositor_CumulativeStats* pStats, uint32_t nStatsSizeInBytes) {
	STUBBED();
}

void CVRCompositor_020::GetCumulativeStats(vr::IVRCompositor_020::Compositor_CumulativeStats* pStats, uint32_t nStatsSizeInBytes) {
	STUBBED();
}

void CVRCompositor_019::GetCumulativeStats(vr::IVRCompositor_019::Compositor_CumulativeStats* pStats, uint32_t nStatsSizeInBytes) {
	STUBBED();
}
