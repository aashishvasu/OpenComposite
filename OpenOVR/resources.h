#pragma once

// Other defines...

#define RES_T_OBJ 256
#define RES_T_PNG 257
#define RES_T_FNTMETA 258
#define RES_T_KBLAYOUT 259

#define RES_O_HAND_LEFT 1
#define RES_O_HAND_RIGHT 2

// Fonts
#define RES_O_FNT_UBUNTU 3

// Keyboard layouts
#define RES_O_KB_EN_GB 4

// Resource list, used on Linux
// clang-format off
#define RES_LIST_LINUX(f) \
	f(RES_O_HAND_LEFT) \
	f(RES_O_HAND_RIGHT) \
	f(RES_O_FNT_UBUNTU) \
	f(RES_O_KB_EN_GB) // clang-format on
