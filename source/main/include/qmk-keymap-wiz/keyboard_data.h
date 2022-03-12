#ifndef __QMK_KEYMAP_WIZ_KEYBOARD_H__
#define __QMK_KEYMAP_WIZ_KEYBOARD_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE
#pragma once
#endif

#include "xbase/x_allocator.h"

struct ImVec4;

namespace xcore
{
    struct key_resource_t
    {
        key_resource_t()
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

    struct keygroup_resource_t
    {
        inline keygroup_resource_t()
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

        const char*     m_name; // name of this group
        float           m_x;    // x position of this group
        float           m_y;    // y position of this group
        float           m_w;    // key width
        float           m_h;    // key height
        float           m_sw;   // key spacing width
        float           m_sh;   // key spacing height
        xcore::s16      m_r;    // rows
        xcore::s16      m_c;    // columns
        xcore::s16      m_a;    // angle, -45 degrees to 45 degrees (granularity is 1 degree)
        xcore::s8       m_capcolor_size;
        xcore::s8       m_txtcolor_size;
        xcore::s8       m_ledcolor_size;
        u8*             m_capcolor; // color of the key cap
        u8*             m_txtcolor; // color of the key label
        u8*             m_ledcolor; // color of the key led
        xcore::s16      m_nb_keys;  // number of keys in the array
        key_resource_t* m_keys;     // array of keys
    };

    struct keyboard_resource_t
    {
        keyboard_resource_t();

        XCORE_CLASS_PLACEMENT_NEW_DELETE

        const char* m_name; // name of this keyboard

        xcore::s16           m_nb_keygroups;
        keygroup_resource_t* m_keygroups;

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

    struct keyboards_resource_t
    {
        keyboards_resource_t()
        {
            m_nb_keyboards = 0;
            m_keyboards    = nullptr;
        }

        XCORE_CLASS_PLACEMENT_NEW_DELETE

        xcore::s32           m_nb_keyboards;
        keyboard_resource_t* m_keyboards;
    };

    void get_color(ImVec4 const& c, u8* color);

    void init_keyboards();
    bool load_keyboards(keyboards_resource_t const*& kbs);
    bool reload_keyboards(keyboards_resource_t const*& kbs);

    // When the user has loaded the keyboard definitions and has selected a keyboard we should also load the mutable data.
    // That data consists of a very simple list of layers.
    enum elayer_mod
    {
        MO  = 0x01, // While held, MOmentarily switch to YOUR_LAYER.
        LT  = 0x02, // Layer Tap. When held: go to YOUR_LAYER. When tapped: send KC_XXXX
        TG  = 0x04, // Layer Toggle. When tapped, toggles YOUR_LAYER on or off
        TO  = 0x08, // When tapped, goes to YOUR_LAYER
        TT  = 0x10, // When tapped, toggles YOUR_LAYER on or off. When held, activates YOUR_LAYER.
        OSL = 0x20,
    };

    enum emod
    {
        LSFT = 0x01, // applies left Shift to KC_XXXX (keycode) - S is an alias
        RSFT = 0x02, // applies right Shift to KC_XXXX
        LCTL = 0x04, // applies left Control to KC_XXXX
        RCTL = 0x08, // applies right Control to KC_XXXX
        LALT = 0x10, // applies left Alt to KC_XXXX
        RALT = 0x20, // applies right Alt to KC_XXXX
        LGUI = 0x40, // applies left GUI (command/win) to KC_XXXX
        RGUI = 0x80, // applies right GUI (command/win) to KC_XXXX
        MT   = 0x100,
        OSM  = 0x200,
    };

    struct key_t
    {
        xcore::u16 m_keycode;
        xcore::u16 m_mod;
        xcore::u16 m_layer_mod;
        xcore::u16 m_layer;
        xcore::u8  m_capcolor[4];
        xcore::u8  m_ledcolor[4];
    };

    struct layer_t
    {
        const char* m_name;
        xcore::s16  m_index;
        xcore::s16  m_nb_keys;
        key_t*      m_keys;
    };

    struct keymap_t
    {
        keymap_t()
        {
            m_nb_layers = 0;
            m_layers    = nullptr;
        }

        XCORE_CLASS_PLACEMENT_NEW_DELETE

        xcore::s32 m_nb_layers;
        layer_t*   m_layers;
    };

    // Also we have 2 databases used for rendering the keyboards:
    // - array of keycode strings to text
    // - array of keycode strings to icons
    struct keycode_t
    {
        xcore::s32  m_id;   // 0
        const char* m_icon; //
        const char* m_code; // "KC_A"
        const char* m_text; // "a"
    };

    struct keycodes_t
    {
    };

} // namespace xcore

#endif // __QMK_KEYMAP_WIZ_KEYBOARD_H__