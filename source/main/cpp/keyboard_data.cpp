#include "xbase/x_base.h"
#include "xbase/x_context.h"
#include "xbase/x_memory.h"

#include "xjson/x_json_decode.h"
#include "xjson/x_json_allocator.h"

#include "qmk-keymap-wiz/keyboard_data.h"

#include "libimgui/imgui.h"

#include <stdio.h>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

namespace xcore
{
    static ckey_t s_default_ckey;

    // clang-format off
    static json::JsonFieldDescr s_members_ckey[] = {
        json::JsonFieldDescr("nob", s_default_ckey.m_nob), 
        json::JsonFieldDescr("index", s_default_ckey.m_index), 
        json::JsonFieldDescr("label", s_default_ckey.m_label), 
        json::JsonFieldDescr("w", s_default_ckey.m_w), 
        json::JsonFieldDescr("h", s_default_ckey.m_h),
        json::JsonFieldDescr("sw", s_default_ckey.m_sw), 
        json::JsonFieldDescr("sh", s_default_ckey.m_sh),
        json::JsonFieldDescr("cap_color", s_default_ckey.m_capcolor, s_default_ckey.m_capcolor_size),
        json::JsonFieldDescr("txt_color", s_default_ckey.m_txtcolor, s_default_ckey.m_txtcolor_size),
        json::JsonFieldDescr("led_color", s_default_ckey.m_ledcolor, s_default_ckey.m_ledcolor_size),
    };

    static void json_alloc_ckey(json::JsonAllocator* alloc, s32 n, void*& ptr) 
    {
        ptr = nullptr;
        if (n == 1) {
            ptr = alloc->Allocate<ckey_t>();
            new (ptr) ckey_t();
        } else if (n > 1) {
            ptr = alloc->AllocateArray<ckey_t>(n); 
            
            char* mem = (char*)ptr;
            for (s32 i = 0; i < n; ++i)
            {
                new (mem + i * sizeof(ckey_t)) ckey_t();
            }            
        }
    }
    static void json_copy_ckey(void* dst, s32 dst_index, void* src ) { ((ckey_t*)dst)[dst_index] = *(ckey_t*)src; }

    static json::JsonTypeDescr json_ckey = 
    {
        "key",
        &s_default_ckey, 
        sizeof(ckey_t),
        ALIGNOF(ckey_t),
        sizeof(s_members_ckey) / sizeof(json::JsonFieldDescr), 
        s_members_ckey
    };

    static json::JsonTypeFuncs json_ckeys_funcs = {
        json_alloc_ckey,
        json_copy_ckey,
    };
    // clang-format on

    static ckeygroup_t s_default_ckeygroup;

    // clang-format off
    static json::JsonFieldDescr s_members_ckeygroup[] = {
        json::JsonFieldDescr("name", s_default_ckeygroup.m_name), 
        json::JsonFieldDescr("x", s_default_ckeygroup.m_x), 
        json::JsonFieldDescr("y", s_default_ckeygroup.m_y),
        json::JsonFieldDescr("w", s_default_ckeygroup.m_w), 
        json::JsonFieldDescr("h", s_default_ckeygroup.m_h), 
        json::JsonFieldDescr("sw", s_default_ckeygroup.m_sw), 
        json::JsonFieldDescr("sh", s_default_ckeygroup.m_sh), 
        json::JsonFieldDescr("r", s_default_ckeygroup.m_r),
        json::JsonFieldDescr("c", s_default_ckeygroup.m_c),
        json::JsonFieldDescr("a", s_default_ckeygroup.m_a),
        json::JsonFieldDescr("cap_color", s_default_ckeygroup.m_capcolor, s_default_ckeygroup.m_capcolor_size), 
        json::JsonFieldDescr("txt_color", s_default_ckeygroup.m_txtcolor, s_default_ckeygroup.m_txtcolor_size), 
        json::JsonFieldDescr("led_color", s_default_ckeygroup.m_ledcolor, s_default_ckeygroup.m_ledcolor_size), 
        json::JsonFieldDescr("keys", s_default_ckeygroup.m_keys, s_default_ckeygroup.m_nb_keys, json_ckeys_funcs, json_ckey), 
    };

    static void json_alloc_ckeygroup(json::JsonAllocator* alloc, s32 n, void*& ptr) 
    { 
        ptr = nullptr;
        if (n == 1) {
            ptr = alloc->Allocate<ckeygroup_t>();
            new (ptr) ckeygroup_t();
        } else if (n > 1) {
            ptr = alloc->AllocateArray<ckeygroup_t>(n);             
            char* mem = (char*)ptr;
            for (s32 i = 0; i < n; ++i)
            {
                new (mem + i * sizeof(ckeygroup_t)) ckeygroup_t();
            }            
        }
    }
    static void json_copy_ckeygroup(void* dst, s32 dst_index, void* src) { ((ckeygroup_t*)dst)[dst_index] = *(ckeygroup_t*)src; }

    static json::JsonTypeDescr json_ckeygroup = {
        "keygroup",
        &s_default_ckeygroup, 
        sizeof(ckeygroup_t),
        ALIGNOF(ckeygroup_t),
        sizeof(s_members_ckeygroup) / sizeof(json::JsonFieldDescr), 
        s_members_ckeygroup
    };

    static json::JsonTypeFuncs json_ckeygroup_funcs = {
        json_alloc_ckeygroup,
        json_copy_ckeygroup,
    };
    // clang-format on

    static const u8 sColorDarkGrey[] = {25, 25, 25, 255};
    static const u8 sColorWhite[]    = {255, 255, 255, 255};
    static const u8 sColorBlue[]     = {0, 0, 255, 255};

    ckeyboard_t::ckeyboard_t()
    {
        m_name = "";

        m_nb_keygroups = 0;
        m_keygroups    = nullptr;

        // global caps, txt and led color, can be overriden per key
        copy(m_capcolor, sColorDarkGrey);
        copy(m_txtcolor, sColorWhite);
        copy(m_ledcolor, sColorBlue);
        m_scale = 80.0f;
        m_w     = 1.0f;
        m_h     = 1.0f;
        m_sw    = 0.0625f;
        m_sh    = 0.0625f;
    }

    static ckeyboard_t s_default_ckeyboard;

    // clang-format off
    static json::JsonFieldDescr s_members_ckeyboard[] = {
        json::JsonFieldDescr("name", s_default_ckeyboard.m_name),
        json::JsonFieldDescr("scale", s_default_ckeyboard.m_scale), 
        json::JsonFieldDescr("key_width", s_default_ckeyboard.m_w), 
        json::JsonFieldDescr("key_height", s_default_ckeyboard.m_h), 
        json::JsonFieldDescr("key_spacing_x", s_default_ckeyboard.m_sw), 
        json::JsonFieldDescr("key_spacing_y", s_default_ckeyboard.m_sh), 
        json::JsonFieldDescr("cap_color", s_default_ckeyboard.m_capcolor, 4), 
        json::JsonFieldDescr("txt_color", s_default_ckeyboard.m_txtcolor, 4), 
        json::JsonFieldDescr("led_color", s_default_ckeyboard.m_ledcolor, 4), 
        json::JsonFieldDescr("keygroups", s_default_ckeyboard.m_keygroups, s_default_ckeyboard.m_nb_keygroups, json_ckeygroup_funcs, json_ckeygroup), 
    };
    // clang-format on

    // implementation of the constructor for the keygroup object
    static void json_construct_ckeyboard(json::JsonAllocator* alloc, s32 n, void*& ptr)
    {
        ptr = nullptr;
        if (n == 1)
        {
            ptr = alloc->Allocate<ckeyboard_t>();
            new (ptr) ckeyboard_t();
        }
        else if (n > 1)
        {
            ptr = alloc->AllocateArray<ckeyboard_t>(n);

            char* mem = (char*)ptr;
            for (s32 i = 0; i < n; ++i)
            {
                new (mem + i * sizeof(ckeyboard_t)) ckeyboard_t();
            }
        }
    }

    static void json_copy_ckeyboard(void* dst, s32 dst_index, void* src) { ((ckeyboard_t*)dst)[dst_index] = *(ckeyboard_t*)src; }

    // clang-format off
    static json::JsonTypeDescr json_ckeyboard = 
    {
        "keyboard",
        &s_default_ckeyboard, 
        sizeof(ckeyboard_t),
        ALIGNOF(ckeyboard_t),
        sizeof(s_members_ckeyboard) / sizeof(json::JsonFieldDescr), 
        s_members_ckeyboard
    };

    static json::JsonTypeFuncs json_ckeyboard_funcs = {
        json_construct_ckeyboard,
        json_copy_ckeyboard,
    };

    static ckeyboards_t s_default_ckeyboards;

    static json::JsonFieldDescr s_members_keyboard_root[] = {
        json::JsonFieldDescr("keyboards", s_default_ckeyboards.m_keyboards, s_default_ckeyboards.m_nb_keyboards, json_ckeyboard_funcs, json_ckeyboard), 
    };
    static json::JsonTypeDescr json_ckeyboards = {
        "keyboards", 
        &s_default_ckeyboards, 
        sizeof(ckeyboards_t),
        ALIGNOF(ckeyboards_t),
        sizeof(s_members_keyboard_root) / sizeof(json::JsonFieldDescr), 
        s_members_keyboard_root
    };
    // clang-format on

    void get_color(ImVec4 const& c, u8* color)
    {
        color[0] = (xcore::u8)(c.x * 255.0f);
        color[1] = (xcore::u8)(c.y * 255.0f);
        color[2] = (xcore::u8)(c.z * 255.0f);
        color[3] = (xcore::u8)(c.w * 255.0f);
    }

    struct skeyboards_t
    {
        skeyboards_t()
            : kbdb_filename("kbdb/kb.json")
            , main_allocator_size(1024 * 1024)
            , scratch_allocator_size(1024 * 1024)
        {

            main1_allocator_memory   = nullptr;
            main2_allocator_memory   = nullptr;
            scratch_allocator_memory = nullptr;
            main_allocator_memory    = nullptr;
        }

        const char* const kbdb_filename;
        xcore::u32 const  main_allocator_size;
        xcore::u32 const  scratch_allocator_size;
        void*             main1_allocator_memory;
        void*             main2_allocator_memory;
        void*             scratch_allocator_memory;
        void*             main_allocator_memory;

        struct stat kbdb_file_state;
        time_t      last_file_poll;
    };

    skeyboards_t s_kbds;

    void init_keyboards()
    {
        // Allocate memory for the main and scratch allocators
        //
        s_kbds.last_file_poll = time(nullptr);

        s_kbds.main1_allocator_memory   = ::malloc(s_kbds.main_allocator_size);
        s_kbds.main2_allocator_memory   = ::malloc(s_kbds.main_allocator_size);
        s_kbds.scratch_allocator_memory = ::malloc(s_kbds.scratch_allocator_size);
        s_kbds.main_allocator_memory    = s_kbds.main1_allocator_memory;
    }

    bool load_keyboards(ckeyboards_t const*& kbs)
    {
        stat(s_kbds.kbdb_filename, &s_kbds.kbdb_file_state);

        // load the file fully in memory
        // open the file
        FILE* f = fopen(s_kbds.kbdb_filename, "rb");
        if (!f)
        {
            printf("failed to open file %s\n", s_kbds.kbdb_filename);
            return false;
        }

        // get the file size
        fseek(f, 0, SEEK_END);
        s32 kbdb_json_len = ftell(f);
        fseek(f, 0, SEEK_SET);

        json::JsonAllocator alloc;
        alloc.Init(s_kbds.main_allocator_memory, s_kbds.main_allocator_size, "JSON allocator");

        json::JsonAllocator scratch;
        scratch.Init(s_kbds.scratch_allocator_memory, s_kbds.scratch_allocator_size, "JSON scratch allocator");

        // allocate the buffer
        char* kbdb_json = scratch.AllocateArray<char>(kbdb_json_len + 1);
        if (!kbdb_json)
        {
            printf("failed to allocate %d bytes for the keyboard json\n", kbdb_json_len);
            alloc.Reset();
            scratch.Reset();
            fclose(f);
            return false;
        }

        // read the file
        fread(kbdb_json, 1, kbdb_json_len, f);
        fclose(f);

        ckeyboards_t* kb = alloc.Allocate<ckeyboards_t>();
        new (kb) ckeyboards_t();

        json::JsonObject json_root;
        json_root.m_descr    = &json_ckeyboards;
        json_root.m_instance = kb;

        char const* error_message = nullptr;
        bool        ok            = json::JsonDecode((const char*)kbdb_json, (const char*)kbdb_json + kbdb_json_len, json_root, &alloc, &scratch, error_message);

        scratch.Reset();
        kbs = kb;
        return ok;
    }

    bool reload_keyboards(xcore::ckeyboards_t const*& kbs)
    {
        // only check every second on the clock if the file has changed
        time_t now = time(nullptr);
        if (now - s_kbds.last_file_poll > 1)
        {
            struct stat kbdb_file_state_updated;
            if (stat(s_kbds.kbdb_filename, &kbdb_file_state_updated) == 0)
            {
                if (kbdb_file_state_updated.st_mtime > s_kbds.kbdb_file_state.st_mtime)
                {
                    if (s_kbds.main_allocator_memory == s_kbds.main1_allocator_memory)
                    {
                        s_kbds.main_allocator_memory = s_kbds.main2_allocator_memory;
                    }
                    else
                    {
                        s_kbds.main_allocator_memory = s_kbds.main1_allocator_memory;
                    }

                    s_kbds.kbdb_file_state = kbdb_file_state_updated;

                    ckeyboards_t const* kbs_reloaded = nullptr;
                    if (load_keyboards(kbs_reloaded))
                    {
                        kbs = kbs_reloaded;
                        return true;
                    }
                }
            }
            s_kbds.last_file_poll = time(nullptr);
        }
        return false;
    }

    // --------------------------------------------------------------------------------------------------------------------------
    // --------------------------------------------------------------------------------------------------------------------------

    key_t::key_t()
    {
        m_keycode_str = "KC_NO";
        m_keycode_idx = 0;
        m_mod         = 0;
        m_layer_mod   = 0;
        m_layer       = 0;
        copy(m_capcolor, sColorDarkGrey);
        copy(m_ledcolor, sColorBlue);
    }

    // clang-format off
    static key_t s_default_key;

    static json::JsonFieldDescr s_members_key[] = {
        json::JsonFieldDescr("keycode", s_default_key.m_keycode_str),
        json::JsonFieldDescr("mod", s_default_key.m_mod),
    };

    static void json_alloc_key(json::JsonAllocator* alloc, s32 n, void*& ptr) 
    {
        ptr = nullptr;
        if (n == 1) {
            ptr = alloc->Allocate<key_t>();
            new (ptr) key_t();
        } else if (n > 1) {
            ptr = alloc->AllocateArray<key_t>(n); 
            
            char* mem = (char*)ptr;
            for (s32 i = 0; i < n; ++i)
            {
                new (mem + i * sizeof(key_t)) key_t();
            }            
        }
    }
    static void json_copy_key(void* dst, s32 dst_index, void* src ) { ((key_t*)dst)[dst_index] = *(key_t*)src; }

    static json::JsonTypeDescr json_key = 
    {
        "key",
        &s_default_key, 
        sizeof(key_t),
        ALIGNOF(key_t),
        sizeof(s_members_key) / sizeof(json::JsonFieldDescr), 
        s_members_key
    };

    static json::JsonTypeFuncs json_key_funcs = {
        json_alloc_key,
        json_copy_key,
    };
    // clang-format on

    // clang-format off
    static layer_t s_default_layer;

    static json::JsonFieldDescr s_members_layer[] = {
        json::JsonFieldDescr("name", s_default_layer.m_name),
        json::JsonFieldDescr("index", s_default_layer.m_index),
        json::JsonFieldDescr("keys", s_default_layer.m_keys, s_default_layer.m_nb_keys, json_key_funcs, json_key),
    };

    static void json_alloc_layer(json::JsonAllocator* alloc, s32 n, void*& ptr) 
    {
        ptr = nullptr;
        if (n == 1) {
            ptr = alloc->Allocate<layer_t>();
            new (ptr) layer_t();
        } else if (n > 1) {
            ptr = alloc->AllocateArray<layer_t>(n); 
            
            char* mem = (char*)ptr;
            for (s32 i = 0; i < n; ++i)
            {
                new (mem + i * sizeof(layer_t)) layer_t();
            }            
        }
    }
    static void json_copy_layer(void* dst, s32 dst_index, void* src ) { ((layer_t*)dst)[dst_index] = *(layer_t*)src; }

    static json::JsonTypeDescr json_layer = 
    {
        "key",
        &s_default_layer, 
        sizeof(layer_t),
        ALIGNOF(layer_t),
        sizeof(s_members_layer) / sizeof(json::JsonFieldDescr), 
        s_members_layer
    };

    static json::JsonTypeFuncs json_layer_funcs = {
        json_alloc_layer,
        json_copy_layer,
    };
    // clang-format on






    // --------------------------------------------------------------------------------------------------------------------------
    // --------------------------------------------------------------------------------------------------------------------------
    // clang-format off
    static keycode_t s_default_keycode;

    static json::JsonFieldDescr s_members_keycode[] = {
        json::JsonFieldDescr("code", s_default_keycode.m_code),
        json::JsonFieldDescr("codes", s_default_keycode.m_codes, s_default_keycode.m_nb_codes),
        json::JsonFieldDescr("normal", s_default_keycode.m_normal),
        json::JsonFieldDescr("shifted", s_default_keycode.m_shifted),
        json::JsonFieldDescr("icon", s_default_keycode.m_icon),
        json::JsonFieldDescr("descr", s_default_keycode.m_descr)
    };

    static void json_alloc_keycode(json::JsonAllocator* alloc, s32 n, void*& ptr) 
    {
        ptr = nullptr;
        if (n == 1) {
            ptr = alloc->Allocate<keycode_t>();
            new (ptr) keycode_t();
        } else if (n > 1) {
            ptr = alloc->AllocateArray<keycode_t>(n); 
            
            char* mem = (char*)ptr;
            for (s32 i = 0; i < n; ++i)
            {
                new (mem + i * sizeof(keycode_t)) keycode_t();
            }            
        }
    }
    static void json_copy_keycode(void* dst, s32 dst_index, void* src ) { ((keycode_t*)dst)[dst_index] = *(keycode_t*)src; }

    static json::JsonTypeDescr json_keycode = 
    {
        "keycode",
        &s_default_keycode, 
        sizeof(keycode_t),
        ALIGNOF(keycode_t),
        sizeof(s_members_keycode) / sizeof(json::JsonFieldDescr), 
        s_members_keycode
    };

    static json::JsonTypeFuncs json_keycode_funcs = {
        json_alloc_keycode,
        json_copy_keycode,
    };
    // clang-format on

    // clang-format off
    static keycodes_t s_default_keycodes;

    static json::JsonFieldDescr s_members_keycodes[] = {
        json::JsonFieldDescr("keycodes", s_default_keycodes.m_keycodes, s_default_keycodes.m_nb_keycodes, json_keycode_funcs, json_keycode), 
    };

    static void json_alloc_keycodes(json::JsonAllocator* alloc, s32 n, void*& ptr) 
    {
        ptr = nullptr;
        if (n == 1) {
            ptr = alloc->Allocate<keycodes_t>();
            new (ptr) keycodes_t();
        } else if (n > 1) {
            ptr = alloc->AllocateArray<keycodes_t>(n); 
            
            char* mem = (char*)ptr;
            for (s32 i = 0; i < n; ++i)
            {
                new (mem + i * sizeof(keycodes_t)) keycodes_t();
            }            
        }
    }
    static void json_copy_keycodes(void* dst, s32 dst_index, void* src ) { ((keycodes_t*)dst)[dst_index] = *(keycodes_t*)src; }

    static json::JsonTypeDescr json_keycodes = 
    {
        "keycodes",
        &s_default_keycodes, 
        sizeof(keycodes_t),
        ALIGNOF(keycodes_t),
        sizeof(s_members_keycode) / sizeof(json::JsonFieldDescr), 
        s_members_keycodes
    };

    static json::JsonTypeFuncs json_keycodes_funcs = {
        json_alloc_keycodes,
        json_copy_keycodes,
    };
    // clang-format on

    struct keycodes_rt_t
    {
        keycodes_rt_t()
            : kbdb_filename("kbdb/keycodes.json")
            , main_allocator_size(4 * 1024 * 1024)
            , scratch_allocator_size(4 * 1024 * 1024)
        {

            main1_allocator_memory   = nullptr;
            main2_allocator_memory   = nullptr;
            scratch_allocator_memory = nullptr;
            main_allocator_memory    = nullptr;
        }

        const char* const kbdb_filename;
        xcore::u32 const  main_allocator_size;
        xcore::u32 const  scratch_allocator_size;
        void*             main1_allocator_memory;
        void*             main2_allocator_memory;
        void*             scratch_allocator_memory;
        void*             main_allocator_memory;

        struct stat kbdb_file_state;
        time_t      last_file_poll;
    };

    static keycodes_rt_t g_kcdb;

    void init_keycodes()
    {
        // Allocate memory for the main and scratch allocators
        //
        g_kcdb.last_file_poll = time(nullptr);

        g_kcdb.main1_allocator_memory   = ::malloc(g_kcdb.main_allocator_size);
        g_kcdb.main2_allocator_memory   = ::malloc(g_kcdb.main_allocator_size);
        g_kcdb.scratch_allocator_memory = ::malloc(g_kcdb.scratch_allocator_size);
        g_kcdb.main_allocator_memory    = g_kcdb.main1_allocator_memory;
    }

    bool load_keycodes(keycodes_t const*& _kcds)
    {
        stat(g_kcdb.kbdb_filename, &g_kcdb.kbdb_file_state);

        // load the file fully in memory
        // open the file
        FILE* f = fopen(g_kcdb.kbdb_filename, "rb");
        if (!f)
        {
            printf("failed to open file %s\n", g_kcdb.kbdb_filename);
            return false;
        }

        // get the file size
        fseek(f, 0, SEEK_END);
        s32 const kcds_json_len = ftell(f);
        fseek(f, 0, SEEK_SET);

        json::JsonAllocator alloc;
        alloc.Init(g_kcdb.main_allocator_memory, g_kcdb.main_allocator_size, "JSON allocator");

        json::JsonAllocator scratch;
        scratch.Init(g_kcdb.scratch_allocator_memory, g_kcdb.scratch_allocator_size, "JSON scratch allocator");

        // allocate the buffer
        char* kcds_json = scratch.AllocateArray<char>(kcds_json_len + 1);
        if (!kcds_json)
        {
            printf("failed to allocate %d bytes for the keyboard json\n", kcds_json_len);
            alloc.Reset();
            scratch.Reset();
            fclose(f);
            return false;
        }

        // read the file
        fread(kcds_json, 1, kcds_json_len, f);
        fclose(f);

        keycodes_t* kcds = alloc.Allocate<keycodes_t>();
        new (kcds) keycodes_t();

        json::JsonObject json_root;
        json_root.m_descr    = &json_ckeyboards;
        json_root.m_instance = kcds;

        char const* error_message = nullptr;
        bool        ok            = json::JsonDecode((const char*)kcds_json, (const char*)kcds_json + kcds_json_len, json_root, &alloc, &scratch, error_message);

        scratch.Reset();
        _kcds = kcds;
        return ok;
    }

} // namespace xcore

using namespace xcore;
