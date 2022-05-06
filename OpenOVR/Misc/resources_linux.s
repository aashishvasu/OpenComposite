// Embed some binary assets since there's no standardised resource compiler in the Linux world
#include "resources.h"

.section .rodata

#define FILENAME_RES_O_HAND_LEFT "assets/LeftHand.obj"
#define FILENAME_RES_O_HAND_RIGHT "assets/RightHand.obj"
#define FILENAME_RES_O_FNT_UBUNTU "assets/Ubuntu-30.sfn"
#define FILENAME_RES_O_KB_EN_GB "assets/en_gb.kb"
// RES_O_FNT_UBUNTU	RES_T_PNG	"assets/Ubuntu-30-texture.png"

// TODO do we need an alignment directive?

#define ADD_RESOURCE(sym)   \
	.global resource_##sym; \
	.global resource_##sym##_end; \
	resource_##sym:; \
	.incbin FILENAME_##sym; \
	resource_##sym##_end:; \
	.byte 0; /* To make this a C string, after end to not affect the length */ \

RES_LIST_LINUX(ADD_RESOURCE)
