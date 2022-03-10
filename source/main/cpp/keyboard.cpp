#include "xbase/x_base.h"
#include "xbase/x_memory.h"

#include "xjson/x_json_decode.h"
#include "xjson/x_json_allocator.h"
#include "qmk-keymap-wiz/keyboard.h"

#include <stdio.h>

namespace xcore
{
    static key_t s_default_key;

    // clang-format off
    static json::JsonFieldDescr s_members_key[] = {
        json::JsonFieldDescr("nob", s_default_key.m_nob), 
        json::JsonFieldDescr("index", s_default_key.m_index), 
        json::JsonFieldDescr("label", s_default_key.m_label), 
        json::JsonFieldDescr("w", s_default_key.m_w), 
        json::JsonFieldDescr("h", s_default_key.m_h),
        json::JsonFieldDescr("sw", s_default_key.m_sw), 
        json::JsonFieldDescr("sh", s_default_key.m_sh),
        json::JsonFieldDescr("cap_color", s_default_key.m_capcolor, s_default_key.m_capcolor_size),
        json::JsonFieldDescr("txt_color", s_default_key.m_txtcolor, s_default_key.m_txtcolor_size),
        json::JsonFieldDescr("led_color", s_default_key.m_ledcolor, s_default_key.m_ledcolor_size),
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

    static json::JsonTypeFuncs json_keys_funcs = {
        json_alloc_key,
        json_copy_key,
    };
    // clang-format on

    static keygroup_t s_default_keygroup;

    // clang-format off
    static json::JsonFieldDescr s_members_keygroup[] = {
        json::JsonFieldDescr("name", s_default_keygroup.m_name), 
        json::JsonFieldDescr("x", s_default_keygroup.m_x), 
        json::JsonFieldDescr("y", s_default_keygroup.m_y),
        json::JsonFieldDescr("w", s_default_keygroup.m_w), 
        json::JsonFieldDescr("h", s_default_keygroup.m_h), 
        json::JsonFieldDescr("sw", s_default_keygroup.m_sw), 
        json::JsonFieldDescr("sh", s_default_keygroup.m_sh), 
        json::JsonFieldDescr("r", s_default_keygroup.m_r),
        json::JsonFieldDescr("c", s_default_keygroup.m_c),
        json::JsonFieldDescr("a", s_default_keygroup.m_a),
        json::JsonFieldDescr("cap_color", s_default_keygroup.m_capcolor, s_default_keygroup.m_capcolor_size), 
        json::JsonFieldDescr("txt_color", s_default_keygroup.m_txtcolor, s_default_keygroup.m_txtcolor_size), 
        json::JsonFieldDescr("led_color", s_default_keygroup.m_ledcolor, s_default_keygroup.m_ledcolor_size), 
        json::JsonFieldDescr("keys", s_default_keygroup.m_keys, s_default_keygroup.m_nb_keys, json_keys_funcs, json_key), 
    };

    static void json_alloc_keygroup(json::JsonAllocator* alloc, s32 n, void*& ptr) 
    { 
        ptr = nullptr;
        if (n == 1) {
            ptr = alloc->Allocate<keygroup_t>();
            new (ptr) keygroup_t();
        } else if (n > 1) {
            ptr = alloc->AllocateArray<keygroup_t>(n);             
            char* mem = (char*)ptr;
            for (s32 i = 0; i < n; ++i)
            {
                new (mem + i * sizeof(keygroup_t)) keygroup_t();
            }            
        }
    }
    static void json_copy_keygroup(void* dst, s32 dst_index, void* src) { ((keygroup_t*)dst)[dst_index] = *(keygroup_t*)src; }

    static json::JsonTypeDescr json_keygroup = {
        "keygroup",
        &s_default_keygroup, 
        sizeof(keygroup_t),
        ALIGNOF(keygroup_t),
        sizeof(s_members_keygroup) / sizeof(json::JsonFieldDescr), 
        s_members_keygroup
    };

    static json::JsonTypeFuncs json_keygroup_funcs = {
        json_alloc_keygroup,
        json_copy_keygroup,
    };
    // clang-format on

    static const u8 sColorDarkGrey[] = {25, 25, 25, 255};
    static const u8 sColorWhite[]    = {255, 255, 255, 255};
    static const u8 sColorBlue[]     = {0, 0, 255, 255};

    keyboard_t::keyboard_t()
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

    static keyboard_t s_default_keyboard;

    // clang-format off
    static json::JsonFieldDescr s_members_keyboard[] = {
        json::JsonFieldDescr("name", s_default_keyboard.m_name),
        json::JsonFieldDescr("scale", s_default_keyboard.m_scale), 
        json::JsonFieldDescr("key_width", s_default_keyboard.m_w), 
        json::JsonFieldDescr("key_height", s_default_keyboard.m_h), 
        json::JsonFieldDescr("key_spacing_x", s_default_keyboard.m_sw), 
        json::JsonFieldDescr("key_spacing_y", s_default_keyboard.m_sh), 
        json::JsonFieldDescr("cap_color", s_default_keyboard.m_capcolor, 4), 
        json::JsonFieldDescr("txt_color", s_default_keyboard.m_txtcolor, 4), 
        json::JsonFieldDescr("led_color", s_default_keyboard.m_ledcolor, 4), 
        json::JsonFieldDescr("keygroups", s_default_keyboard.m_keygroups, s_default_keyboard.m_nb_keygroups, json_keygroup_funcs, json_keygroup), 
    };
    // clang-format on

    // implementation of the constructor for the keygroup object
    static void json_construct_keyboard(json::JsonAllocator* alloc, s32 n, void*& ptr)
    {
        ptr = nullptr;
        if (n == 1)
        {
            ptr = alloc->Allocate<keyboard_t>();
            new (ptr) keyboard_t();
        }
        else if (n > 1)
        {
            ptr = alloc->AllocateArray<keyboard_t>(n);

            char* mem = (char*)ptr;
            for (s32 i = 0; i < n; ++i)
            {
                new (mem + i * sizeof(keyboard_t)) keyboard_t();
            }
        }
    }

    static void json_copy_keyboard(void* dst, s32 dst_index, void* src) { ((keyboard_t*)dst)[dst_index] = *(keyboard_t*)src; }

    // clang-format off
    static json::JsonTypeDescr json_keyboard = 
    {
        "keyboard",
        &s_default_keyboard, 
        sizeof(keyboard_t),
        ALIGNOF(keyboard_t),
        sizeof(s_members_keyboard) / sizeof(json::JsonFieldDescr), 
        s_members_keyboard
    };

    static json::JsonTypeFuncs json_keyboard_funcs = {
        json_construct_keyboard,
        json_copy_keyboard,
    };

    static keyboards_t s_default_keyboards;

    static json::JsonFieldDescr s_members_keyboard_root[] = {
        json::JsonFieldDescr("keyboards", s_default_keyboards.m_keyboards, s_default_keyboards.m_nb_keyboards, json_keyboard_funcs, json_keyboard), 
    };
    static json::JsonTypeDescr json_keyboards = {
        "keyboards", 
        &s_default_keyboards, 
        sizeof(keyboards_t),
        ALIGNOF(keyboards_t),
        sizeof(s_members_keyboard_root) / sizeof(json::JsonFieldDescr), 
        s_members_keyboard_root
    };
    // clang-format on

    keyboards_t* load_keyboards(void* mem_buffer, u32 mem_buffer_len, void* scratch_buffer, u32 scratch_buffer_len, const char* filename)
    {
        // load the file fully in memory
        // open the file
        FILE* f = fopen(filename, "rb");
        if (!f)
        {
            printf("failed to open file %s\n", filename);
            return nullptr;
        }

        // get the file size
        fseek(f, 0, SEEK_END);
        s32 kbdb_json_len = ftell(f);
        fseek(f, 0, SEEK_SET);

        json::JsonAllocator alloc;
        alloc.Init(mem_buffer, mem_buffer_len, "JSON allocator");

        json::JsonAllocator scratch;
        scratch.Init(scratch_buffer, scratch_buffer_len, "JSON scratch allocator");

        // allocate the buffer
        char* kbdb_json = scratch.AllocateArray<char>(kbdb_json_len + 1);
        if (!kbdb_json)
        {
            printf("failed to allocate %d bytes for the keyboard json\n", kbdb_json_len);
            alloc.Reset();
            scratch.Reset();
            return nullptr;
        }

        // read the file
        fread(kbdb_json, 1, kbdb_json_len, f);

        keyboards_t* kb = alloc.Allocate<keyboards_t>();
        new (kb) keyboards_t();

        json::JsonObject json_root;
        json_root.m_descr    = &json_keyboards;
        json_root.m_instance = kb;

        char const* error_message = nullptr;
        bool        ok            = json::JsonDecode((const char*)kbdb_json, (const char*)kbdb_json + kbdb_json_len, json_root, &alloc, &scratch, error_message);

        scratch.Reset();
        return kb;
    }

} // namespace xcore

using namespace xcore;
