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
    struct ckey_t
    {
        ckey_t()
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

    struct ckeygroup_t
    {
        inline ckeygroup_t()
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
        ckey_t* m_keys;     // array of keys
    };

    struct ckeyboard_t
    {
        ckeyboard_t();

        XCORE_CLASS_PLACEMENT_NEW_DELETE

        const char* m_name; // name of this keyboard

        xcore::s16           m_nb_keygroups;
        ckeygroup_t* m_keygroups;

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

    struct ckeyboards_t
    {
        ckeyboards_t()
        {
            m_nb_keyboards = 0;
            m_keyboards    = nullptr;
        }

        XCORE_CLASS_PLACEMENT_NEW_DELETE

        xcore::s32           m_nb_keyboards;
        ckeyboard_t* m_keyboards;
    };

    void get_color(ImVec4 const& c, u8* color);

    void init_keyboards();
    bool load_keyboards(ckeyboards_t const*& kbs);
    bool reload_keyboards(ckeyboards_t const*& kbs);

    // -----------------------------------------------------------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------
    // When the user has loaded the keyboard definitions and has selected a keyboard we should also load the mutable data.
    // That data consists of a very simple list of layers.
    enum elayer_switch
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
    };

    enum emod_tap 
    {
        MT   = 0x100,// 'mod' when held, 'keycode' when tapped 
        OSM  = 0x200, // one-shot modifier
    };

    struct key_t
    {
        key_t();

        XCORE_CLASS_PLACEMENT_NEW_DELETE

        const char* m_keycode_str;
        xcore::u16  m_mod;
        xcore::u16  m_mod_tap;
        xcore::u16  m_layer_switch;
        const char* m_layer;
        xcore::u8   m_capcolor[4];
        xcore::u8   m_ledcolor[4];
    };

    struct layer_t
    {
        layer_t();

        XCORE_CLASS_PLACEMENT_NEW_DELETE

        const char* m_name;
        xcore::s16  m_index;
        xcore::s16  m_nb_keys;
        key_t*      m_keys;
        xcore::u8   m_capcolor[4];
        xcore::u8   m_ledcolor[4];
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

    // -----------------------------------------------------------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------
    // Also we have 2 databases used for rendering the keyboards:
    // - array of keycode strings to text
    // - array of keycode strings to icons
    struct keycode_t
    {
        keycode_t()
        {
            m_code     = nullptr;
            m_nb_codes = 0;
            m_codes    = nullptr;
            m_normal   = nullptr;
            m_shifted  = nullptr;
            m_icon     = nullptr;
            m_descr    = nullptr;
        }

        XCORE_CLASS_PLACEMENT_NEW_DELETE

        const char*  m_code;     // "KC_ESC"
        xcore::s32   m_nb_codes; // 2
        const char** m_codes;    // ["KC_ESCAPE", "KC_ESC"]
        const char*  m_normal;   // Normal keycode text
        const char*  m_shifted;  // Shifted keycode text
        const char*  m_icon;     // Unicode characters that represent an icon
        const char*  m_descr;    // A description of the keycode
    };

    struct keycodes_t
    {
        keycodes_t()
        {
            m_nb_keycodes = 0;
            m_keycodes    = nullptr;
        }

        XCORE_CLASS_PLACEMENT_NEW_DELETE

        xcore::s32 m_nb_keycodes;
        keycode_t* m_keycodes;
    };

    void init_keycodes();
    bool load_keycodes(keycodes_t const*& _kcds);

} // namespace xcore

#endif // __QMK_KEYMAP_WIZ_KEYBOARD_H__