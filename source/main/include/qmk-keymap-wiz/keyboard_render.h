#ifndef __QMK_KEYMAP_WIZ_KEYBOARD_RENDER_H__
#define __QMK_KEYMAP_WIZ_KEYBOARD_RENDER_H__
#pragma once

#include "qmk-keymap-wiz/keyboard_data.h"

void keyboard_render(xcore::ckeyboard_t const* kb, xcore::keycodes_t const* kcdb, xcore::keymap_t const* km, xcore::s32 layer, float posx, float posy, float mousex, float mousey, float globalscale);
void keyboard_loadfonts();

#endif // __QMK_KEYMAP_WIZ_KEYBOARD_RENDER_H__