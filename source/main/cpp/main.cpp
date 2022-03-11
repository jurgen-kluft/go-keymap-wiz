#include "xbase/x_base.h"
#include "xbase/x_context.h"
#include "xbase/x_memory.h"
#include "qmk-keymap-wiz/keyboard.h"

#include "libimgui/imgui.h"
#include "libimgui/imgui_internal.h"
#include "libimgui/imgui_impl_glfw.h"
#include "libimgui/imgui_impl_opengl3.h"

#include <stdio.h>
#include <math.h> // sqrtf, powf, cosf, sinf, floorf, ceilf
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>

#include "libglfw/glfw3.h" // Will drag in system OpenGL headers

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

ImFont* Fonts[] = { 
    nullptr,
    nullptr,
    nullptr,
    nullptr,
 };

static ImVec4 Darken(ImVec4 const& c, float p)
{
    ImVec4 r;
    r.x = c.x - (c.x * p);
    r.y = c.y - (c.y * p);
    r.z = c.z - (c.z * p);
    r.w = c.w - (c.w * p);
    return r;
}

static void GetColor(ImVec4& c, xcore::u8* color)
{
    color[0] = (xcore::u8)(c.x * 255.0f);
    color[1] = (xcore::u8)(c.y * 255.0f);
    color[2] = (xcore::u8)(c.z * 255.0f);
    color[3] = (xcore::u8)(c.w * 255.0f);
}

ImVec2 operator-(const ImVec2& l, const ImVec2& r) { return {l.x - r.x, l.y - r.y}; }
struct ImRotation
{
    ImRotation(ImDrawList* draw_list)
    {
        m_draw_list = draw_list;
        m_start     = draw_list->VtxBuffer.Size;
    }

    ImVec2 Center()
    {
        ImVec2 l(FLT_MAX, FLT_MAX), u(-FLT_MAX, -FLT_MAX); // bounds

        const auto& buf = m_draw_list->VtxBuffer;
        for (int i = m_start; i < buf.Size; i++)
            l = ImMin(l, buf[i].pos), u = ImMax(u, buf[i].pos);

        return ImVec2((l.x + u.x) / 2, (l.y + u.y) / 2); // or use _ClipRectStack?
    }

    void Apply(float rad)
    {
        ImVec2 center = Center();

        float s = (float)sin(rad), c = (float)cos(rad);
        center = ImRotate(center, c, s) - center;

        auto& buf = m_draw_list->VtxBuffer;
        for (int i = m_start; i < buf.Size; i++)
            buf[i].pos = ImRotate(buf[i].pos, c, s) - center;
    }

    ImDrawList* m_draw_list;
    int         m_start;
};

static bool IsInsideKey(xcore::keyboard_t const* kb, xcore::keygroup_t const& kg, xcore::key_t const& key, float globalscale, float x, float y, float px, float py)
{
    const float sw        = key.m_sw * kg.m_sw * kb->m_sw * kb->m_scale * globalscale;
    const float sh        = key.m_sh * kg.m_sh * kb->m_sh * kb->m_scale * globalscale;
    const float kw        = (key.m_w * kg.m_w * kb->m_w * kb->m_scale * globalscale) - (2 * sw);
    const float kh        = (key.m_h * kg.m_h * kb->m_h * kb->m_scale * globalscale) - (2 * sh);
    const float hw        = kw / 2;
    const float hh        = kh / 2;
    if (px >= (x - hw) && px <= (x + hw) && py >= (y - hh) && py <= (y + hh))
        return true;
    return false;
}

static ImVec2 SelectFontSize(const char* txt, float kw)
{
    ImGui::PushFont(Fonts[0]);
    ImVec2 td = ImGui::CalcTextSize(txt);
    if ((td.x * 1.1f) >= kw)
    {
        ImGui::PopFont();

        ImGui::PushFont(Fonts[1]);
        td = ImGui::CalcTextSize(txt);
        if ((td.x * 1.1f) >= kw)
        {
            ImGui::PopFont();

            ImGui::PushFont(Fonts[2]);
            td = ImGui::CalcTextSize(txt);
            if ((td.x * 1.1f) >= kw)
            {
                ImGui::PopFont();

                ImGui::PushFont(Fonts[3]);
                td = ImGui::CalcTextSize(txt);
            }
        }
    }
    return td;
}

static void DrawKey(xcore::keyboard_t const* kb, xcore::keygroup_t const& kg, xcore::key_t const& key, float globalscale, float x, float y, float r, bool highlight)
{
    if (highlight)
        globalscale *= 1.2f;

    const float sw        = key.m_sw * kg.m_sw * kb->m_sw * kb->m_scale * globalscale;
    const float sh        = key.m_sh * kg.m_sh * kb->m_sh * kb->m_scale * globalscale;
    const float kw        = (key.m_w * kg.m_w * kb->m_w * kb->m_scale * globalscale) - (2 * sw);
    const float kh        = (key.m_h * kg.m_h * kb->m_h * kb->m_scale * globalscale) - (2 * sh);
    const float rounding  = kw / sw;
    const float hw        = kw / 2;
    const float hh        = kh / 2;
    const float th        = 10.0f * globalscale;

    const xcore::u8* capcolor = kb->m_capcolor;
    const xcore::u8* txtcolor = kb->m_txtcolor;
    const xcore::u8* ledcolor = kb->m_ledcolor;

    if (kg.m_capcolor)
        capcolor = kg.m_capcolor;
    if (kg.m_txtcolor)
        txtcolor = kg.m_txtcolor;
    if (kg.m_ledcolor)
        ledcolor = kg.m_ledcolor;

    if (key.m_capcolor)
        capcolor = key.m_capcolor;
    if (key.m_txtcolor)
        txtcolor = key.m_txtcolor;
    if (key.m_ledcolor)
        ledcolor = key.m_ledcolor;

    const ImU32 rkeycapcolor = ImColor(capcolor);
    ImVec4      dkeyledcolor(ledcolor);
    const ImU32 rkeyledcolor1 = ImColor(Darken(dkeyledcolor, 0.2f));
    const ImU32 rkeyledcolor2 = ImColor(Darken(dkeyledcolor, 0.1f));
    const ImU32 rkeyledcolor3 = ImColor(ledcolor);
    const ImU32 rkeytxtcolor  = ImColor(txtcolor);

    // draw_list->AddRect(ImVec2(x, y), ImVec2(x + sz, y + sz), rkeyledcolor, rounding, ImDrawFlags_RoundCornersAll, th/1.0f);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImRotation  rotation(draw_list);

    draw_list->AddRect(ImVec2(x - hw, y - hh), ImVec2(x + hw, y + hh), (rkeyledcolor1), rounding, ImDrawFlags_None, th / 1.0f);
    draw_list->AddRect(ImVec2(x - hw, y - hh), ImVec2(x + hw, y + hh), (rkeyledcolor2), rounding, ImDrawFlags_None, th / 2.0f);
    draw_list->AddRect(ImVec2(x - hw, y - hh), ImVec2(x + hw, y + hh), (rkeyledcolor3), rounding, ImDrawFlags_None, th / 3.0f);
    draw_list->AddRectFilled(ImVec2(x - hw, y - hh), ImVec2(x + hw, y + hh), rkeycapcolor, rounding, ImDrawFlags_RoundCornersAll);

    if (key.m_label != nullptr)
    {
        /*
        < ,
         
        */
       char   text[128];
       char* lines[4] = { nullptr, nullptr, nullptr, nullptr };
        

        int num_lines = 0;
        int i = 0;
        lines[num_lines++] = text;
        while (key.m_label[i] != 0)
        {
            if (key.m_label[i] == ' ')
            {
                text[i] = 0;
                lines[num_lines++] = (text + i + 1);
                text[i+1] = 0;
            } else {
                text[i] = key.m_label[i];
                text[i+1] = 0;
            }
            i++;
        }

        if (num_lines == 1)
        {
            ImVec2 td = SelectFontSize(key.m_label, kw);
            draw_list->AddText(ImVec2(x - (td.x / 2), y - (td.y / 2)), rkeytxtcolor, key.m_label);
            ImGui::PopFont();
        }
        else if (num_lines == 2)
        {
            // Push/Pop another smaller font
            ImVec2 td1 = SelectFontSize(lines[0], kw);
            draw_list->AddText(ImVec2(x - (td1.x / 2), y - (td1.y * 1.1f)), rkeytxtcolor, lines[0]);
            ImGui::PopFont();

            ImVec2 td2 = SelectFontSize(lines[1], kw);
            draw_list->AddText(ImVec2(x - (td2.x / 2), y + (td2.y * 0.1f)), rkeytxtcolor, lines[1]);
            ImGui::PopFont();
        }
    }

    if (key.m_nob)
        draw_list->AddLine(ImVec2(x - (hw*0.125), y + 0.5f * hh), ImVec2(x + (hw*0.125), y + 0.5f * hh), ImColor(255, 255, 255, 255), 2);

    rotation.Apply(r);
}

void render(xcore::keyboard_t const* kb, float posx, float posy, float mousex, float mousey, float globalscale)
{
    // origin = left/top corner
    float ox = posx + (kb->m_w / 2);
    float oy = posy + (kb->m_h / 2);

    int highlighted_key_index = -1;

    for (int g = 0; g < kb->m_nb_keygroups; g++)
    {
        xcore::keygroup_t const& kg = kb->m_keygroups[g];

        float gx = ox + (kg.m_x * kb->m_scale * globalscale);
        float gy = oy + (kg.m_y * kb->m_scale * globalscale);

        ImVec2 ydir(0.0f, 1.0f); // down
        ImVec2 xdir(1.0f, 0.0f); // right

        ImVec2 mdir(mousex-gx, mousey-gy);

        float rrad = 0.0f;
        if (kg.m_a > 0 || kg.m_a < 0)
        {
            // convert degrees 'm_r' to radians
            rrad = (float)3.141592653f * kg.m_a / 180.0f;
            // rotate dir by kg->m_r
            const float s = (float)sin(rrad);
            const float c = (float)cos(rrad);
            ydir          = ImRotate(ydir, c, s);
            xdir          = ImRotate(xdir, c, s);
        }

        if (kg.m_a > 0 || kg.m_a < 0)
        {
            float rot = (float)-3.141592653f * kg.m_a / 180.0f;
            const float s = (float)sin(rot);
            const float c = (float)cos(rot);
            mdir          = ImRotate(mdir, c, s);
        }

        float mx = gx + mdir.x;
        float my = gy + mdir.y;

        float rx = gx;
        float ry = gy;
        int k = 0;
        for (int r = 0; r < kg.m_r && highlighted_key_index == -1; r++)
        {
            float cx = rx;
            float cy = ry;

            for (int c = 0; c < kg.m_c && highlighted_key_index == -1; c++)
            {
                xcore::key_t const& kc = kg.m_keys[k++];
                if (IsInsideKey(kb, kg, kc, globalscale, cx, cy, mx, my))
                {
                    highlighted_key_index = kc.m_index;
                    break;
                }
                cx += (kb->m_w * kg.m_w) * kb->m_scale * globalscale;
            }

            ry += (kb->m_h * kg.m_h) * kb->m_scale * globalscale;
        }
 
        rx = gx;
        ry = gy;
        k = 0;
        for (int r = 0; r < kg.m_r; r++)
        {
            float cx = rx;
            float cy = ry;

            for (int c = 0; c < kg.m_c; c++)
            {
                xcore::key_t const& kc = kg.m_keys[k++];
                DrawKey(kb, kg, kc, globalscale, cx, cy, rrad, kc.m_index == highlighted_key_index);

                cx += xdir.x * (kb->m_w * kg.m_w) * kb->m_scale * globalscale;
                cy += xdir.y * (kb->m_h * kg.m_h) * kb->m_scale * globalscale;
            }

            rx += ydir.x * (kb->m_w * kg.m_w) * kb->m_scale * globalscale;
            ry += ydir.y * (kb->m_h * kg.m_h) * kb->m_scale * globalscale;
        }

    }
}


// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

static void glfw_error_callback(int error, const char* description) { fprintf(stderr, "Glfw Error %d: %s\n", error, description); }

namespace xcore
{
    class WizAssertHandler : public xcore::asserthandler_t
    {
    public:
        WizAssertHandler() { NumberOfAsserts = 0; }

        virtual bool handle_assert(u32& flags, const char* fileName, s32 lineNumber, const char* exprString, const char* messageString)
        {
            NumberOfAsserts++;
            return false;
        }

        xcore::s32 NumberOfAsserts;
    };
} // namespace xcore

static xcore::WizAssertHandler gAssertHandler;

int main(int, char**)
{
    xbase::init();

#ifdef TARGET_DEBUG
    xcore::context_t::set_assert_handler(&gAssertHandler);
#endif

    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

        // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    // glfwWindowHint(GLFW_DECORATED, GL_FALSE);

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(2048, 1024, "QMK Keymap Wizard", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which
    // ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    
    io.Fonts->AddFontDefault();

    static const ImWchar icon_ranges[] =
    {
        0xf000, 0xf3ff,
        0,
    };
    static const ImWchar symbol_ranges[] =
    {
        0x2300, 0x23ff,
        0,
    };
    static const ImWchar base_ranges[] =
    {
        0x0020, 0x00ff,
        0,
    };

    ImFontConfig config;
    config.MergeMode = true;    

    Fonts[0] = io.Fonts->AddFontFromFileTTF("fonts/Roboto-Medium.ttf", 28.0f, nullptr, &base_ranges[0]);
    io.Fonts->AddFontFromFileTTF("fonts/freeserif.ttf", 34.0f, &config, &symbol_ranges[0]);
    io.Fonts->AddFontFromFileTTF("fonts/fontawesome-webfont.ttf", 28.0f, &config, &icon_ranges[0]);
    io.Fonts->Build();

    Fonts[1] = io.Fonts->AddFontFromFileTTF("fonts/Roboto-Medium.ttf", 24.0f, nullptr, &base_ranges[0]);
    io.Fonts->AddFontFromFileTTF("fonts/freeserif.ttf", 24.0f, &config, &symbol_ranges[0]);
    io.Fonts->AddFontFromFileTTF("fonts/fontawesome-webfont.ttf", 24.0f, &config, &icon_ranges[0]);
    io.Fonts->Build();

    Fonts[2] = io.Fonts->AddFontFromFileTTF("fonts/Roboto-Medium.ttf", 20.0f, nullptr, &base_ranges[0]);
    io.Fonts->AddFontFromFileTTF("fonts/freeserif.ttf", 20.0f, &config, &symbol_ranges[0]);
    io.Fonts->AddFontFromFileTTF("fonts/fontawesome-webfont.ttf", 20.0f, &config, &icon_ranges[0]);
    io.Fonts->Build();

    Fonts[3] = io.Fonts->AddFontFromFileTTF("fonts/Roboto-Medium.ttf", 16.0f, nullptr, &base_ranges[0]);
    io.Fonts->AddFontFromFileTTF("fonts/freeserif.ttf", 16.0f, &config, &symbol_ranges[0]);
    io.Fonts->AddFontFromFileTTF("fonts/fontawesome-webfont.ttf", 16.0f, &config, &icon_ranges[0]);
    io.Fonts->Build();

    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    // IM_ASSERT(font != NULL);

    // Our state
    bool   show_demo_window    = true;
    bool   show_another_window = false;
    ImVec4 clear_color         = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Allocate memory for the main and scratch allocators
    //
    xcore::u32 const main_allocator_size      = 1024 * 1024;
    xcore::u32 const scratch_allocator_size   = 1024 * 1024;
    void*     main1_allocator_memory    = malloc(main_allocator_size);
    void*     main2_allocator_memory    = malloc(main_allocator_size);
    void*     scratch_allocator_memory = malloc(scratch_allocator_size);
    void*     main_allocator_memory    = main1_allocator_memory;

    const char* kbdb_filename = "kbdb/kb.json";
    
    // see if file has changed
    struct stat kbdb_file_state;
    stat(kbdb_filename, &kbdb_file_state);

    xcore::keyboards_t* keebs = xcore::load_keyboards(main_allocator_memory, main_allocator_size, scratch_allocator_memory, scratch_allocator_size, "kbdb/kb.json");
    xcore::keyboard_t* kb = &keebs->m_keyboards[0];

    // Main loop

    time_t last_file_poll = time(nullptr);   
    while (!glfwWindowShouldClose(window))
    {
        // only check every second on the clock if the file has changed
        time_t now = time(nullptr);
        if (now - last_file_poll > 1)
        {
            struct stat kbdb_file_state_updated;
            if (stat(kbdb_filename, &kbdb_file_state_updated) == 0)
            {
                if (kbdb_file_state_updated.st_mtime > kbdb_file_state.st_mtime)
                {
                    if (main_allocator_memory == main1_allocator_memory)
                    {
                        main_allocator_memory = main2_allocator_memory;
                    }
                    else
                    {
                        main_allocator_memory = main1_allocator_memory;
                    }

                    kbdb_file_state = kbdb_file_state_updated;
                    xcore::keyboards_t* keebs_updated = xcore::load_keyboards(main_allocator_memory, main_allocator_size, scratch_allocator_memory, scratch_allocator_size, kbdb_filename);
                    if (keebs_updated!=nullptr)
                    {
                        keebs = keebs_updated;
                        kb = &keebs->m_keyboards[0];
                    }
                }
            }
            last_file_poll = time(nullptr);   
        }

        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImVec2 winSize;

        {
            static float f       = 0.0f;
            static int   counter = 0;

            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::Begin("Keyboard Wiz", nullptr,
                         ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

            ImGui::BeginChildFrame(7, ImVec2(320, 200), 0);

            ImGui::Text("counter = %d", counter);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            const float MIN_SCALE = 0.7f;
            const float MAX_SCALE = 1.5f;
            ImGui::DragFloat("global scale", &io.FontGlobalScale, 0.005f, MIN_SCALE, MAX_SCALE, "%.2f", ImGuiSliderFlags_AlwaysClamp); // Scale everything

            static ImGuiColorEditFlags alpha_flags = 0;
            static ImVec4              keycapcolor = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
            ImGui::ColorEdit4("key cap color", (float*)&keycapcolor, ImGuiColorEditFlags_AlphaBar | alpha_flags);

            static ImVec4 keyledcolor = ImVec4(1.0f, 0.1f, 0.1f, 1.0f);
            ImGui::ColorEdit4("key led color", (float*)&keyledcolor, ImGuiColorEditFlags_AlphaBar | alpha_flags);

            GetColor(keycapcolor, kb->m_capcolor);
            GetColor(keyledcolor, kb->m_ledcolor);

            ImGui::EndChildFrame();

            // Tip: If you do a lot of custom rendering, you probably want to use your own geometrical types and benefit of
            // overloaded operators, etc. Define IM_VEC2_CLASS_EXTRA in imconfig.h to create implicit conversions between your
            // types and ImVec2/ImVec4. Dear ImGui defines overloaded operators but they are internal to imgui.cpp and not
            // exposed outside (to avoid messing with your types) In this example we are not using the maths operators!

            if (ImGui::BeginTabBar("##TabBar"))
            {
                ImColor tabkgrndcolor(10,10,10,256);

                if (ImGui::BeginTabItem("QWERTY Layer"))
                {
                    ImGui::BeginChildFrame(8, ImVec2(2048, 840), 0);

                    ImVec2 p = ImGui::GetCursorScreenPos();

                    ImDrawList* draw_list = ImGui::GetWindowDrawList();
                    draw_list->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + 2048, p.y + 800), tabkgrndcolor, 0, ImDrawFlags_RoundCornersAll);

                    render(kb, p.x, p.y, io.MousePos.x, io.MousePos.y, io.FontGlobalScale);

                    ImGui::EndChildFrame();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("SYM Layer"))
                {
                    ImGui::BeginChildFrame(8, ImVec2(2048, 840), 0);

                    ImVec2 p = ImGui::GetCursorScreenPos();

                    ImDrawList* draw_list = ImGui::GetWindowDrawList();
                    draw_list->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + 2048, p.y + 800), tabkgrndcolor, 0, ImDrawFlags_RoundCornersAll);

                    render(kb, p.x, p.y, io.MousePos.x, io.MousePos.y, io.FontGlobalScale);

                    ImGui::EndChildFrame();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("RAISE Layer"))
                {
                    ImGui::BeginChildFrame(8, ImVec2(2048, 840), 0);

                    ImVec2 p = ImGui::GetCursorScreenPos();

                    ImDrawList* draw_list = ImGui::GetWindowDrawList();
                    draw_list->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + 2048, p.y + 800), tabkgrndcolor, 0, ImDrawFlags_RoundCornersAll);

                    render(kb, p.x, p.y, io.MousePos.x, io.MousePos.y, io.FontGlobalScale);

                    ImGui::EndChildFrame();
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
            winSize = ImGui::GetWindowSize();
            ImGui::End();
        }

        if (winSize.x < 2048)
            winSize.x = 2048;
        if (winSize.y < 1024)
            winSize.y = 1024;

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        // glfwSetWindowPos( window, ((int)winSize.x) / 2, ((int)winSize.y) / 2 );
        glfwSetWindowSize(window, (int)winSize.x, (int)winSize.y); // Resize
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}