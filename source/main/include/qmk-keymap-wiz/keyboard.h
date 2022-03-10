#ifndef __QMK_KEYMAP_WIZ_KEYBOARD_H__
#define __QMK_KEYMAP_WIZ_KEYBOARD_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

#include "xbase/x_allocator.h"

namespace xcore
{
    struct key_t
    {
        key_t()
        {
            m_nob           = false;
            m_index         = -1;
            m_label         = "Q";
            m_w             = 1.0f;
            m_h             = 1.0f;
            m_sw            = 1.0f;
            m_sh            = 1.0f;
            m_capcolor_size = 0;
            m_txtcolor_size = 0;
            m_ledcolor_size = 0;
            m_capcolor      = nullptr;
            m_ledcolor      = nullptr;
            m_txtcolor      = nullptr;
        }

        XCORE_CLASS_PLACEMENT_NEW_DELETE

        bool        m_nob;           // home-key (e.g. the F or J key)
        xcore::s16  m_index;         // index in keymap
        const char* m_label;         // label (e.g. "Q")
        float       m_w;             // key width
        float       m_h;             // key height
        float       m_sw;            // key spacing w
        float       m_sh;            // key spacing h
        xcore::s8   m_capcolor_size; // should become 3 or 4 (RGB or RGBA)
        xcore::s8   m_txtcolor_size; //
        xcore::s8   m_ledcolor_size; //
        u8*         m_capcolor;      // color of the key cap, e.g. [ 0.1, 0.1, 0.1, 1.0 ]
        u8*         m_txtcolor;      // color of the key label
        u8*         m_ledcolor;      // color of the key led
    };

    struct keygroup_t
    {
        inline keygroup_t()
        {
            m_name          = "";
            m_x             = 0.0f;
            m_y             = 0.0f;
            m_w             = 1.0f;
            m_h             = 1.0f;
            m_sw            = 1.0f;
            m_sh            = 1.0f;
            m_r             = 1;
            m_c             = 1;
            m_a             = 0;
            m_capcolor_size = 0;
            m_txtcolor_size = 0;
            m_ledcolor_size = 0;
            m_capcolor      = nullptr;
            m_txtcolor      = nullptr;
            m_ledcolor      = nullptr;
            m_nb_keys       = 0;
            m_keys          = nullptr;
        }

        XCORE_CLASS_PLACEMENT_NEW_DELETE

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
        u8*         m_capcolor; // color of the key cap
        u8*         m_txtcolor; // color of the key label
        u8*         m_ledcolor; // color of the key led
        xcore::s16  m_nb_keys;  // number of keys in the array
        key_t*      m_keys;     // array of keys
    };

    struct keyboard_t
    {
        keyboard_t();

        XCORE_CLASS_PLACEMENT_NEW_DELETE

        const char* m_name; // name of this keyboard

        xcore::s16  m_nb_keygroups;
        keygroup_t* m_keygroups;

        // global caps, txt and led color, can be overriden per key
        u8    m_capcolor[4];
        u8    m_txtcolor[4];
        u8    m_ledcolor[4];
        float m_scale;
        float m_w;  // key width
        float m_h;  // key height
        float m_sw; // key spacing width
        float m_sh; // key spacing height
    };

    struct keyboards_t
    {
        keyboards_t()
        {
            m_nb_keyboards = 0;
            m_keyboards    = nullptr;
        }

        XCORE_CLASS_PLACEMENT_NEW_DELETE

        xcore::s32  m_nb_keyboards;
        keyboard_t* m_keyboards;
    };

    keyboards_t* load_keyboards(void* mem_buffer, u32 mem_buffer_len, void* scratch_buffer, u32 scratch_buffer_len, const char* filename);

} // namespace xcore

#endif // __QMK_KEYMAP_WIZ_KEYBOARD_H__