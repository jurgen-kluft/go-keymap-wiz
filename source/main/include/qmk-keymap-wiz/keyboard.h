#ifndef __QMK_KEYMAP_WIZ_KEYBOARD_H__
#define __QMK_KEYMAP_WIZ_KEYBOARD_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

namespace xcore
{
    struct key_t
    {
        key_t()
        {
            m_nob      = false;
            m_index    = 0;
            m_label    = "Q";
            m_w        = 80.0f;
            m_h        = 80.0f;
            m_capcolor = nullptr;
            m_ledcolor = nullptr;
            m_txtcolor = nullptr;
        }

        bool        m_nob;           // home-key (e.g. the F or J key)
        xcore::s16  m_index;         // index in keymap
        const char* m_label;         // label (e.g. "Q")
        float       m_w;             // key width
        float       m_h;             // key height
        xcore::s8   m_capcolor_size; // should become 3 or 4 (RGB or RGBA)
        xcore::s8   m_txtcolor_size; //
        xcore::s8   m_ledcolor_size; //
        float*      m_capcolor;      // color of the key cap, e.g. [ 0.1, 0.1, 0.1, 1.0 ]
        float*      m_txtcolor;      // color of the key label
        float*      m_ledcolor;      // color of the key led
    };

    struct keygroup_t
    {
        const char* m_name; // name of this group
        float       m_x;    // x position of this group
        float       m_y;    // y position of this group
        float       m_w;    // key width
        float       m_h;    // key height
        float       m_sw;   // key spacing width
        float       m_sh;   // key spacing height
        xcore::s16  m_r;    // rows
        xcore::s16  m_c;    // columns
        xcore::s16  m_a;    // angle, -45 degrees to 45 degrees (granularity is 1 degree)
        xcore::s8   m_capcolor_size;
        xcore::s8   m_txtcolor_size;
        xcore::s8   m_ledcolor_size;
        float*      m_capcolor; // color of the key cap
        float*      m_txtcolor; // color of the key label
        float*      m_ledcolor; // color of the key led
        xcore::s16  m_nb_keys;  // number of keys in the array
        key_t*      m_keys;     // array of keys
    };


    struct keyboard_t
    {
        keyboard_t();

        const char* m_name; // name of this keyboard

        xcore::s16  m_nb_keygroups;
        keygroup_t* m_keygroups;

        // global caps, txt and led color, can be overriden per key
        float m_capcolor[4];
        float m_txtcolor[4];
        float m_ledcolor[4];
        float m_scale;
        float m_w;  // key width
        float m_h;  // key height
        float m_sw; // key spacing width
        float m_sh; // key spacing height
    };

    struct keyboards_t
    {
        keyboards_t() { m_keyboard = nullptr; }
        xcore::s32 m_nb_keyboards;
        keyboard_t* m_keyboard;
    };

}

#endif // __QMK_KEYMAP_WIZ_KEYBOARD_H__