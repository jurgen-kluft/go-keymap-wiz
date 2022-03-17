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
    template <> void json::JsonObjectTypeRegisterFields<ckey_t>(ckey_t& base, json::JsonFieldDescr*& members, s32& member_count)
    {
        static json::JsonFieldDescr s_members[] = {
            json::JsonFieldDescr("nob", base.m_nob),
            json::JsonFieldDescr("index", base.m_index),
            json::JsonFieldDescr("label", base.m_label),
            json::JsonFieldDescr("w", base.m_w),
            json::JsonFieldDescr("h", base.m_h),
            json::JsonFieldDescr("sw", base.m_sw),
            json::JsonFieldDescr("sh", base.m_sh),
            json::JsonFieldDescr("cap_color", base.m_capcolor, base.m_capcolor_size),
            json::JsonFieldDescr("txt_color", base.m_txtcolor, base.m_txtcolor_size),
            json::JsonFieldDescr("led_color", base.m_ledcolor, base.m_ledcolor_size),
        };
        members      = s_members;
        member_count = sizeof(s_members) / sizeof(json::JsonFieldDescr);
    }
    static json::JsonObjectTypeDeclr<ckey_t> json_ckey("ckey");

    template <> void json::JsonObjectTypeRegisterFields<ckeygroup_t>(ckeygroup_t& base, json::JsonFieldDescr*& members, s32& member_count)
    {
        static json::JsonFieldDescr s_members[] = {
            json::JsonFieldDescr("name", base.m_name),
            json::JsonFieldDescr("x", base.m_x),
            json::JsonFieldDescr("y", base.m_y),
            json::JsonFieldDescr("w", base.m_w),
            json::JsonFieldDescr("h", base.m_h),
            json::JsonFieldDescr("sw", base.m_sw),
            json::JsonFieldDescr("sh", base.m_sh),
            json::JsonFieldDescr("r", base.m_r),
            json::JsonFieldDescr("c", base.m_c),
            json::JsonFieldDescr("a", base.m_a),
            json::JsonFieldDescr("cap_color", base.m_capcolor, base.m_capcolor_size),
            json::JsonFieldDescr("txt_color", base.m_txtcolor, base.m_txtcolor_size),
            json::JsonFieldDescr("led_color", base.m_ledcolor, base.m_ledcolor_size),
            json::JsonFieldDescr("keys", base.m_keys, base.m_nb_keys, json_ckey),
        };
        members      = s_members;
        member_count = sizeof(s_members) / sizeof(json::JsonFieldDescr);
    }
    static json::JsonObjectTypeDeclr<ckeygroup_t> json_ckeygroup("ckeygroup");

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

    template <> void json::JsonObjectTypeRegisterFields<ckeyboard_t>(ckeyboard_t& base, json::JsonFieldDescr*& members, s32& member_count)
    {
        static json::JsonFieldDescr s_members[] = {
            json::JsonFieldDescr("name", base.m_name),
            json::JsonFieldDescr("scale", base.m_scale),
            json::JsonFieldDescr("key_width", base.m_w),
            json::JsonFieldDescr("key_height", base.m_h),
            json::JsonFieldDescr("key_spacing_x", base.m_sw),
            json::JsonFieldDescr("key_spacing_y", base.m_sh),
            json::JsonFieldDescr("cap_color", base.m_capcolor, 4),
            json::JsonFieldDescr("txt_color", base.m_txtcolor, 4),
            json::JsonFieldDescr("led_color", base.m_ledcolor, 4),
            json::JsonFieldDescr("keygroups", base.m_keygroups, base.m_nb_keygroups, json_ckeygroup),
        };
        members      = s_members;
        member_count = sizeof(s_members) / sizeof(json::JsonFieldDescr);
    }
    static json::JsonObjectTypeDeclr<ckeyboard_t> json_ckeyboard("ckeyboard");

    template <> void json::JsonObjectTypeRegisterFields<ckeyboards_t>(ckeyboards_t& base, json::JsonFieldDescr*& members, s32& member_count)
    {
        static json::JsonFieldDescr s_members[] = {
            json::JsonFieldDescr("keyboards", base.m_keyboards, base.m_nb_keyboards, json_ckeyboard),
        };
        members      = s_members;
        member_count = sizeof(s_members) / sizeof(json::JsonFieldDescr);
    }
    static json::JsonObjectTypeDeclr<ckeyboards_t> json_ckeyboards("ckeyboards");

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
            : filename("kbdb/keyboards.json")
            , main_allocator_size(1024 * 1024)
            , scratch_allocator_size(1024 * 1024)
        {

            main1_allocator_memory   = nullptr;
            main2_allocator_memory   = nullptr;
            scratch_allocator_memory = nullptr;
            main_allocator_memory    = nullptr;
        }

        const char* const filename;
        xcore::u32 const  main_allocator_size;
        xcore::u32 const  scratch_allocator_size;
        void*             main1_allocator_memory;
        void*             main2_allocator_memory;
        void*             scratch_allocator_memory;
        void*             main_allocator_memory;

        struct stat file_state;
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

    void exit_keyboards()
    {
        if (s_kbds.main1_allocator_memory)
        {
            ::free(s_kbds.main1_allocator_memory);
            s_kbds.main1_allocator_memory = nullptr;
        }
        if (s_kbds.main2_allocator_memory)
        {
            ::free(s_kbds.main2_allocator_memory);
            s_kbds.main2_allocator_memory = nullptr;
        }
        if (s_kbds.scratch_allocator_memory)
        {
            ::free(s_kbds.scratch_allocator_memory);
            s_kbds.scratch_allocator_memory = nullptr;
        }
    }

    bool load_keyboards(ckeyboards_t const*& kbs)
    {
        stat(s_kbds.filename, &s_kbds.file_state);

        // load the file fully in memory
        // open the file
        FILE* f = fopen(s_kbds.filename, "rb");
        if (!f)
        {
            printf("failed to open file %s\n", s_kbds.filename);
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
            if (stat(s_kbds.filename, &kbdb_file_state_updated) == 0)
            {
                if (kbdb_file_state_updated.st_mtime > s_kbds.file_state.st_mtime)
                {
                    if (s_kbds.main_allocator_memory == s_kbds.main1_allocator_memory)
                    {
                        s_kbds.main_allocator_memory = s_kbds.main2_allocator_memory;
                    }
                    else
                    {
                        s_kbds.main_allocator_memory = s_kbds.main1_allocator_memory;
                    }

                    s_kbds.file_state = kbdb_file_state_updated;

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

    /*
    - Normal
    - Mod Tap
    - Layer Tap
    -

    */

    key_t::key_t()
    {
        m_keycode_str  = "KC_NO";
        m_mod          = 0;
        m_mod_tap      = 0;
        m_layer_switch = 0;
        m_layer        = 0;
        copy(m_capcolor, sColorDarkGrey);
        copy(m_ledcolor, sColorBlue);
    }

    template <> void json::JsonObjectTypeRegisterFields<key_t>(key_t& base, json::JsonFieldDescr*& members, s32& member_count)
    {
        // clang-format off
        static json::JsonFieldDescr s_members[] = 
        {
            json::JsonFieldDescr("keycode", base.m_keycode_str),       
            json::JsonFieldDescr("mod", base.m_mod),     
            json::JsonFieldDescr("mod_tap", base.m_mod_tap),
            json::JsonFieldDescr("layer_switch", base.m_layer_switch), 
            json::JsonFieldDescr("layer", base.m_layer), 
            json::JsonFieldDescr("cap_color", base.m_capcolor, 4),
            json::JsonFieldDescr("led_color", base.m_ledcolor, 4),
        };
        // clang-format on
        members      = s_members;
        member_count = sizeof(s_members) / sizeof(json::JsonFieldDescr);
    }
    static json::JsonObjectTypeDeclr<key_t> json_key("key");

    layer_t::layer_t()
    {
        m_name    = "";
        m_index   = -1;
        m_nb_keys = 0;
        m_keys    = nullptr;
        copy(m_capcolor, sColorDarkGrey);
        copy(m_ledcolor, sColorBlue);
    }

    template <> void json::JsonObjectTypeRegisterFields<layer_t>(layer_t& base, json::JsonFieldDescr*& members, s32& member_count)
    {
        // clang-format off
        static json::JsonFieldDescr s_members[] = {
            json::JsonFieldDescr("name", base.m_name),
            json::JsonFieldDescr("index", base.m_index),
            json::JsonFieldDescr("keys", base.m_keys, base.m_nb_keys, json_key),
        };
        // clang-format on
        members      = s_members;
        member_count = sizeof(s_members) / sizeof(json::JsonFieldDescr);
    }
    static json::JsonObjectTypeDeclr<layer_t> json_layer("layer");

    template <> void json::JsonObjectTypeRegisterFields<keymap_t>(keymap_t& base, json::JsonFieldDescr*& members, s32& member_count)
    {
        // clang-format off
        static json::JsonFieldDescr s_members[] = {
            json::JsonFieldDescr("layers", base.m_layers, base.m_nb_layers, json_layer),
        };
        // clang-format on
        members      = s_members;
        member_count = sizeof(s_members) / sizeof(json::JsonFieldDescr);
    }
    static json::JsonObjectTypeDeclr<keymap_t> json_keymap("keymap");


    template <> void json::JsonObjectTypeRegisterFields<keymaps_t>(keymaps_t& base, json::JsonFieldDescr*& members, s32& member_count)
    {
        // clang-format off
        static json::JsonFieldDescr s_members[] = {
            json::JsonFieldDescr("keymaps", base.m_keymaps, base.m_nb_keymaps, json_keymap),
        };
        // clang-format on
        members      = s_members;
        member_count = sizeof(s_members) / sizeof(json::JsonFieldDescr);
    }
    static json::JsonObjectTypeDeclr<keymaps_t> json_keymaps("keymaps");

    // --------------------------------------------------------------------------------------------------------------------------
    // --------------------------------------------------------------------------------------------------------------------------
    template <> void json::JsonObjectTypeRegisterFields<keycode_t>(keycode_t& base, json::JsonFieldDescr*& members, s32& member_count)
    {
        // clang-format off
        static json::JsonFieldDescr s_members[] = {
            json::JsonFieldDescr("code", base.m_code),
            json::JsonFieldDescr("codes", base.m_codes, base.m_nb_codes),
            json::JsonFieldDescr("normal", base.m_normal),
            json::JsonFieldDescr("shifted", base.m_shifted),
            json::JsonFieldDescr("icon", base.m_icon),
            json::JsonFieldDescr("descr", base.m_descr)
        };
        // clang-format on
        members      = s_members;
        member_count = sizeof(s_members) / sizeof(json::JsonFieldDescr);
    }
    static json::JsonObjectTypeDeclr<keycode_t> json_keycode("keycode");

    template <> void json::JsonObjectTypeRegisterFields<keycodes_t>(keycodes_t& base, json::JsonFieldDescr*& members, s32& member_count)
    {
        // clang-format off
        static json::JsonFieldDescr s_members[] = {
            json::JsonFieldDescr("keycodes", base.m_keycodes, base.m_nb_keycodes, json_keycode), 
        };
        // clang-format on
        members      = s_members;
        member_count = sizeof(s_members) / sizeof(json::JsonFieldDescr);
    }
    static json::JsonObjectTypeDeclr<keycodes_t> json_keycodes("keycodes");

    struct skeycodes_t
    {
        skeycodes_t()
            : filename("kbdb/keycodes.json")
            , main_allocator_size(4 * 1024 * 1024)
            , scratch_allocator_size(4 * 1024 * 1024)
        {
            scratch_allocator_memory = nullptr;
            main_allocator_memory    = nullptr;
        }

        const char* const filename;
        xcore::u32 const  main_allocator_size;
        xcore::u32 const  scratch_allocator_size;
        void*             scratch_allocator_memory;
        void*             main_allocator_memory;

        struct stat file_state;
        time_t      last_file_poll;
    };

    static skeycodes_t s_kcdb;

    void init_keycodes()
    {
        // Allocate memory for the main and scratch allocators
        //
        s_kcdb.last_file_poll = time(nullptr);

        s_kcdb.main_allocator_memory    = ::malloc(s_kcdb.main_allocator_size);
        s_kcdb.scratch_allocator_memory = ::malloc(s_kcdb.scratch_allocator_size);
    }

    void exit_keycodes()
    {
        if (s_kcdb.main_allocator_memory)
        {
            ::free(s_kcdb.main_allocator_memory);
            s_kcdb.main_allocator_memory = nullptr;
        }
        if (s_kcdb.scratch_allocator_memory)
        {
            ::free(s_kcdb.scratch_allocator_memory);
            s_kcdb.scratch_allocator_memory = nullptr;
        }
    }

    bool load_keycodes(keycodes_t const*& _kcds)
    {
        stat(s_kcdb.filename, &s_kcdb.file_state);

        // load the file fully in memory
        // open the file
        FILE* f = fopen(s_kcdb.filename, "rb");
        if (!f)
        {
            printf("failed to open file %s\n", s_kcdb.filename);
            return false;
        }

        // get the file size
        fseek(f, 0, SEEK_END);
        s32 const kcds_json_len = ftell(f);
        fseek(f, 0, SEEK_SET);

        json::JsonAllocator alloc;
        alloc.Init(s_kcdb.main_allocator_memory, s_kcdb.main_allocator_size, "JSON allocator");

        json::JsonAllocator scratch;
        scratch.Init(s_kcdb.scratch_allocator_memory, s_kcdb.scratch_allocator_size, "JSON scratch allocator");

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


    struct skeymaps_t
    {
        skeymaps_t()
            : filename("keymaps/jurgen.json")
            , main_allocator_size(4 * 1024 * 1024)
            , scratch_allocator_size(4 * 1024 * 1024)
        {
            scratch_allocator_memory = nullptr;
            main_allocator_memory    = nullptr;
        }

        const char* const filename;
        xcore::u32 const  main_allocator_size;
        xcore::u32 const  scratch_allocator_size;
        void*             scratch_allocator_memory;
        void*             main_allocator_memory;

        struct stat file_state;
        time_t      last_file_poll;
    };

    static skeymaps_t s_keymaps;

    void init_keymaps()
    {
        // Allocate memory for the main and scratch allocators
        //
        s_keymaps.last_file_poll = time(nullptr);

        s_keymaps.main_allocator_memory    = ::malloc(s_keymaps.main_allocator_size);
        s_keymaps.scratch_allocator_memory = ::malloc(s_keymaps.scratch_allocator_size);
    }

    void exit_keymaps()
    {
        if (s_keymaps.main_allocator_memory)
        {
            ::free(s_keymaps.main_allocator_memory);
            s_keymaps.main_allocator_memory = nullptr;
        }
        if (s_keymaps.scratch_allocator_memory)
        {
            ::free(s_keymaps.scratch_allocator_memory);
            s_keymaps.scratch_allocator_memory = nullptr;
        }
    }

    bool load_keymaps(keymaps_t const*& _keymaps)
    {
        stat(s_keymaps.filename, &s_keymaps.file_state);

        // load the file fully in memory
        // open the file
        FILE* f = fopen(s_keymaps.filename, "rb");
        if (!f)
        {
            printf("failed to open file %s\n", s_keymaps.filename);
            return false;
        }

        // get the file size
        fseek(f, 0, SEEK_END);
        s32 const kcds_json_len = ftell(f);
        fseek(f, 0, SEEK_SET);

        json::JsonAllocator alloc;
        alloc.Init(s_keymaps.main_allocator_memory, s_keymaps.main_allocator_size, "JSON allocator");

        json::JsonAllocator scratch;
        scratch.Init(s_keymaps.scratch_allocator_memory, s_keymaps.scratch_allocator_size, "JSON scratch allocator");

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

        keymaps_t* keymaps = alloc.Allocate<keymaps_t>();
        new (keymaps) keymaps_t();

        json::JsonObject json_root;
        json_root.m_descr    = &json_keymaps;
        json_root.m_instance = keymaps;

        char const* error_message = nullptr;
        bool        ok            = json::JsonDecode((const char*)kcds_json, (const char*)kcds_json + kcds_json_len, json_root, &alloc, &scratch, error_message);

        scratch.Reset();
        _keymaps = keymaps;
        return ok;
    }




} // namespace xcore

using namespace xcore;
