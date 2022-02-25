#include "libimgui/imgui.h"
#include "libimgui/imgui_internal.h"
#include "libimgui/imgui_impl_glfw.h"
#include "libimgui/imgui_impl_opengl3.h"

#include <stdio.h>
#include <math.h> // sqrtf, powf, cosf, sinf, floorf, ceilf

#include "libglfw/glfw3.h" // Will drag system OpenGL headers

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

struct key_t
{
    key_t()
    {
        m_nob      = false;
        m_index    = 0;
        m_label    = "Q";
        m_w        = 80.0f;
        m_h        = 80.0f;
        m_capcolor = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
        m_ledcolor = ImVec4(1.0f, 0.1f, 0.1f, 1.0f);
        m_txtcolor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    }

    bool        m_nob;      // home-key (e.g. the F or J key)
    int         m_index;    // index in keymap
    const char* m_label;    // label (e.g. "Q")
    float       m_w;        // key width
    float       m_h;        // key height
    ImVec4      m_capcolor; // color of the key cap
    ImVec4      m_txtcolor; // color of the key label
    ImVec4      m_ledcolor; // color of the key led
};

struct keygroup_t
{
    const char* m_name;
    bool        m_horizontal; // horizontal or vertical
    float       m_x;
    float       m_y;
    int         m_r;  // -45 degrees to 45 degrees (granularity is 1 degree)
    float       m_w;  // key width
    float       m_h;  // key height
    float       m_sw; // key spacing width
    float       m_sh; // key spacing height
    int         m_nb_keys;
    key_t*      m_keys;
};

struct keyboard_t
{
    keyboard_t()
    {
        m_nb_keygroups = 0;
        m_scale        = 1.0f;
        m_w            = 81.f;
        m_h            = 81.f;
        m_sw           = 9.f;
        m_sh           = 9.f;
    }

    int        m_nb_keygroups;
    keygroup_t m_keygroup[256];

    // global caps, txt and led color, can be overriden per key
    ImVec4 m_capcolor;
    ImVec4 m_txtcolor;
    ImVec4 m_ledcolor;
    float  m_scale;
    float  m_w;  // key width
    float  m_h;  // key height
    float  m_sw; // key spacing width
    float  m_sh; // key spacing height
};

keygroup_t* new_keygroup(keyboard_t& kb, int num_keys)
{
    keygroup_t* k = &kb.m_keygroup[kb.m_nb_keygroups++];

    k->m_name = "";

    k->m_horizontal = true;

    k->m_x = 0.f;
    k->m_y = 0.f;
    k->m_r = 0;

    k->m_w  = kb.m_w;
    k->m_h  = kb.m_h;
    k->m_sw = kb.m_sw;
    k->m_sh = kb.m_sh;

    k->m_nb_keys = num_keys;
    k->m_keys    = new key_t[num_keys];

    return k;
}
keygroup_t* add_keygroup(keyboard_t& kb, int k1)
{
    keygroup_t* k        = new_keygroup(kb, 1);
    k->m_keys[0].m_index = k1;
    return k;
}
keygroup_t* add_keygroup(keyboard_t& kb, int k1, int k2)
{
    keygroup_t* k        = new_keygroup(kb, 2);
    k->m_keys[0].m_index = k1;
    k->m_keys[1].m_index = k2;
    return k;
}
keygroup_t* add_keygroup(keyboard_t& kb, int k1, int k2, int k3)
{
    keygroup_t* k        = new_keygroup(kb, 3);
    k->m_keys[0].m_index = k1;
    k->m_keys[1].m_index = k2;
    k->m_keys[2].m_index = k3;
    return k;
}
keygroup_t* add_keygroup(keyboard_t& kb, int k1, int k2, int k3, int k4)
{
    keygroup_t* k        = new_keygroup(kb, 4);
    k->m_keys[0].m_index = k1;
    k->m_keys[1].m_index = k2;
    k->m_keys[2].m_index = k3;
    k->m_keys[3].m_index = k4;
    return k;
}

static ImVec4 Darken(ImVec4 const& c, float p)
{
    ImVec4 r;
    r.x = c.x - (c.x * p);
    r.y = c.y - (c.y * p);
    r.z = c.z - (c.z * p);
    r.w = c.w - (c.w * p);
    return r;
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

static void DrawKey(keyboard_t const& kb, float cx, float cy, float r, key_t const& key)
{
    const float thickness = 10.0f * kb.m_scale;
    const float kw        = key.m_w * kb.m_scale;
    const float kh        = key.m_h * kb.m_scale;
    const float rounding  = kw / 5.0f;
    const float x         = cx;
    const float y         = cy;
	const float hw = key.m_w / 2;
	const float hh = key.m_h / 2;
    const float th        = thickness;

    const ImU32 rkeycapcolor = ImColor(key.m_capcolor);
    ImVec4      dkeyledcolor(key.m_ledcolor);
    const ImU32 rkeyledcolor1 = ImColor(Darken(key.m_ledcolor, 0.2f));
    const ImU32 rkeyledcolor2 = ImColor(Darken(key.m_ledcolor, 0.1f));
	const ImU32 rkeyledcolor3 = ImColor(key.m_ledcolor);
	const ImU32 rkeytxtcolor = ImColor(key.m_txtcolor);

    // draw_list->AddRect(ImVec2(x, y), ImVec2(x + sz, y + sz), rkeyledcolor, rounding, ImDrawFlags_RoundCornersAll, th/1.0f);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImRotation  rotation(draw_list);

    draw_list->AddRect(ImVec2(x-hw, y-hh), ImVec2(x + hw, y + hh), (rkeyledcolor1), rounding, ImDrawFlags_None, th / 1.0f);
    draw_list->AddRect(ImVec2(x-hw, y-hh), ImVec2(x + hw, y + hh), (rkeyledcolor2), rounding, ImDrawFlags_None, th / 2.0f);
    draw_list->AddRect(ImVec2(x-hw, y-hh), ImVec2(x + hw, y + hh), (rkeyledcolor3), rounding, ImDrawFlags_None, th / 3.0f);
    draw_list->AddRectFilled(ImVec2(x-hw, y-hh), ImVec2(x + hw, y + hh), rkeycapcolor, rounding, ImDrawFlags_RoundCornersAll);

	if (key.m_label != nullptr)
	{
		ImVec2 td = ImGui::CalcTextSize(key.m_label);
		draw_list->AddText(ImVec2(x - (td.x/2), y - (td.y/2)), rkeytxtcolor, key.m_label);
	}

	if (key.m_nob)
		draw_list->AddLine(ImVec2(x - 5, y + 0.5*hh), ImVec2(x + 5, y + 0.5*hh), ImColor(255, 255, 255, 255), 2);

    rotation.Apply(r);
}

/*
{
	"key_width": 80,
	"key_height": 80,
	"key_spacing_x": 10,
	"key_spacing_y": 10,
	"keygroups": [
		{
			"h": false,
			"x": 10,
			"y": 100,
			"rotation": 0,
			"keys": [
				{ "label": "Esc", "index": 0, "home": false, "cap_color": #00FF00, "led_color": #FFFFFF, "txt_color": #FFFFFF, "key_width": 80, "key_height": 80 }
			]
		},

	]
}

*/

// setup a keyboard similar to Kyria
void setup(keyboard_t& kb)
{
    keygroup_t* l1   = add_keygroup(kb, 0, 12, 24);
    l1->m_horizontal = false;
    l1->m_x          = 10.f;
    l1->m_y          = 100.f;

    keygroup_t* l2   = add_keygroup(kb, 1, 13, 25);
    l2->m_horizontal = false;
    l2->m_x          = 10.f + 1 * (kb.m_w + kb.m_sw);
    l2->m_y          = 100.f;
	l2->m_keys[0].m_label = "Q";
	l2->m_keys[1].m_label = "A";
	l2->m_keys[2].m_label = "Z";

    keygroup_t* l3   = add_keygroup(kb, 2, 14, 26);
    l3->m_horizontal = false;
    l3->m_x          = 10.f + 2 * (kb.m_w + kb.m_sw);
    l3->m_y          = 100.f - 60.0f;

    keygroup_t* l4   = add_keygroup(kb, 3, 15, 27);
    l4->m_horizontal = false;
    l4->m_x          = 10.f + 3 * (kb.m_w + kb.m_sw);
    l4->m_y          = 100.f - 90.0f;

    keygroup_t* l5   = add_keygroup(kb, 4, 16, 28);
    l5->m_horizontal = false;
    l5->m_x          = 10.f + 4 * (kb.m_w + kb.m_sw);
    l5->m_y          = 100.f - 60.0f;
	l5->m_keys[1].m_nob = true;

    keygroup_t* l6   = add_keygroup(kb, 5, 17, 29);
    l6->m_horizontal = false;
    l6->m_x          = 10.f + 5 * (kb.m_w + kb.m_sw);
    l6->m_y          = 100.f - 50.0f;

    keygroup_t* l7   = new_keygroup(kb, 2);
    l7->m_horizontal = false;
    l7->m_x          = 10.f + 6 * (kb.m_w + kb.m_sw);
    l7->m_y          = 100.f + 2 * (kb.m_h + kb.m_sh);
    l7->m_r          = 30;
	l7->m_keys[0].m_label = "Q";
	l7->m_keys[1].m_label = "A";

    keygroup_t* l8   = new_keygroup(kb, 2);
    l8->m_horizontal = false;
    l8->m_x          = 10.f + 7 * (kb.m_w + kb.m_sw);
    l8->m_y          = 70.f + 3 * (kb.m_h + kb.m_sh);
    l8->m_r          = 40;
	l8->m_keys[0].m_label = "Q";
	l8->m_keys[1].m_label = "A";

    // horizontal group of 2 keys
    keygroup_t* l11   = new_keygroup(kb, 2);
    l11->m_horizontal = true;
    l11->m_x          = 10.f + (kb.m_w / 2.6f) + 2 * (kb.m_w + kb.m_sw);
    l11->m_y          = 100.f - 60.0f + 3 * (kb.m_w + kb.m_sw);
    ;

    keygroup_t* l0   = new_keygroup(kb, 1);
    l0->m_horizontal = true;
    l0->m_x          = 10.f + (kb.m_w / 2.6f) + kb.m_sw + 4 * (kb.m_w + kb.m_sw);
    l0->m_y          = 100.f - 50.0f + 3 * (kb.m_w + kb.m_sw);
    ;
    l0->m_r = 10;

    keygroup_t* r8   = new_keygroup(kb, 2);
    r8->m_horizontal = false;
    r8->m_x          = 10.f + 9 * (kb.m_w + kb.m_sw);
    r8->m_y          = 70.f + 3 * (kb.m_h + kb.m_sh);
    r8->m_r          = -40;

    keygroup_t* r7   = new_keygroup(kb, 2);
    r7->m_horizontal = false;
    r7->m_x          = 10.f + 10 * (kb.m_w + kb.m_sw);
    r7->m_y          = 100.f + 2 * (kb.m_h + kb.m_sh);
    r7->m_r          = -30;

    keygroup_t* r6   = new_keygroup(kb, 3);
    r6->m_horizontal = false;
    r6->m_x          = 10.f + 11 * (kb.m_w + kb.m_sw);
    r6->m_y          = 100.f - 50.0f;

    keygroup_t* r5   = new_keygroup(kb, 3);
    r5->m_horizontal = false;
    r5->m_x          = 10.f + 12 * (kb.m_w + kb.m_sw);
    r5->m_y          = 100.f - 60.0f;
	r5->m_keys[1].m_nob = true;

    keygroup_t* r4   = new_keygroup(kb, 3);
    r4->m_horizontal = false;
    r4->m_x          = 10.f + 13 * (kb.m_w + kb.m_sw);
    r4->m_y          = 100.f - 90.0f;

    keygroup_t* r3   = new_keygroup(kb, 3);
    r3->m_horizontal = false;
    r3->m_x          = 10.f + 14 * (kb.m_w + kb.m_sw);
    r3->m_y          = 100.f - 60.0f;

    keygroup_t* r2   = new_keygroup(kb, 3);
    r2->m_horizontal = false;
    r2->m_x          = 10.f + 15 * (kb.m_w + kb.m_sw);
    r2->m_y          = 100.f;

    keygroup_t* r1   = new_keygroup(kb, 3);
    r1->m_horizontal = false;
    r1->m_x          = 10.f + 16 * (kb.m_w + kb.m_sw);
    r1->m_y          = 100.f;

    // horizontal group of 2 keys
    keygroup_t* r11   = new_keygroup(kb, 2);
    r11->m_horizontal = true;
    r11->m_x          = 10.f - (kb.m_w / 2.6f) + 13 * (kb.m_w + kb.m_sw);
    r11->m_y          = 100.f - 60.0f + 3 * (kb.m_w + kb.m_sw);
    ;

    keygroup_t* r0   = new_keygroup(kb, 1);
    r0->m_horizontal = true;
    r0->m_x          = 10.f - (kb.m_w / 2.6f) - kb.m_sw + 12 * (kb.m_w + kb.m_sw);
    r0->m_y          = 100.f - 50.0f + 3 * (kb.m_w + kb.m_sw);
    ;
    r0->m_r = -10;
}

void render(ImVec2 const& pos, keyboard_t const& kb)
{
    float cx = pos.x + kb.m_sw + kb.m_w;
    float cy = pos.y + kb.m_sw + kb.m_h;

    // Draw a bunch of primitives
    float ks = kb.m_scale;
    float kw = kb.m_w * ks;
    float kh = kb.m_h * ks;

    for (int g = 0; g < kb.m_nb_keygroups; g++)
    {
        keygroup_t const* kg = &kb.m_keygroup[g];

        float x = cx + kg->m_x;
        float y = cy + kg->m_y;

        ImVec2 dir(0.0f, 0.0f);
        if (kg->m_horizontal)
        {
            dir.x = 1.0f;
        }
        else
        {
            dir.y = 1.0f;
        }

        float rrad = 0.0f;
        if (kg->m_r > 0 || kg->m_r < 0)
        {
            // convert degrees 'm_r' to radians
            rrad = (float)3.141592653f * kg->m_r / 180.0f;
            // rotate dir by kg->m_r
            const float s = (float)sin(rrad);
            const float c = (float)cos(rrad);
            dir           = ImRotate(dir, c, s);
        }

        for (int k = 0; k < kg->m_nb_keys; k++)
        {
            key_t const& key = kg->m_keys[k];
            DrawKey(kb, x - (kw / 2), y - (kh / 2), rrad, key);

            x += dir.x * (kw + kg->m_sw);
            y += dir.y * (kh + kg->m_sh);
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

int main(int, char**)
{
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
    // io.Fonts->AddFontDefault();
    io.Fonts->AddFontFromFileTTF("fonts/Roboto-Medium.ttf", 28.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    // IM_ASSERT(font != NULL);

    // Our state
    bool   show_demo_window    = true;
    bool   show_another_window = false;
    ImVec4 clear_color         = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    static keyboard_t kb;
    setup(kb);
    kb.m_scale = io.FontGlobalScale;

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
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

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f       = 0.0f;
            static int   counter = 0;

            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::Begin("Keyboard Wiz", nullptr,
                         ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

            ImGui::BeginChildFrame(7, ImVec2(320, 200), 0);

            ImGui::Text("counter = %d", counter);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            const float MIN_SCALE = 0.3f;
            const float MAX_SCALE = 2.0f;
            ImGui::DragFloat("global scale", &io.FontGlobalScale, 0.005f, MIN_SCALE, MAX_SCALE, "%.2f", ImGuiSliderFlags_AlwaysClamp); // Scale everything

            static ImGuiColorEditFlags alpha_flags = 0;
            static ImVec4              keycapcolor = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
            ImGui::ColorEdit4("key cap color", (float*)&kb.m_capcolor, ImGuiColorEditFlags_AlphaBar | alpha_flags);

            static ImVec4 keyledcolor = ImVec4(1.0f, 0.1f, 0.1f, 1.0f);
            ImGui::ColorEdit4("key led color", (float*)&kb.m_ledcolor, ImGuiColorEditFlags_AlphaBar | alpha_flags);

            ImGui::EndChildFrame();

            // Tip: If you do a lot of custom rendering, you probably want to use your own geometrical types and benefit of
            // overloaded operators, etc. Define IM_VEC2_CLASS_EXTRA in imconfig.h to create implicit conversions between your
            // types and ImVec2/ImVec4. Dear ImGui defines overloaded operators but they are internal to imgui.cpp and not
            // exposed outside (to avoid messing with your types) In this example we are not using the maths operators!

            if (ImGui::BeginTabBar("##TabBar"))
            {
                if (ImGui::BeginTabItem("QWERTY Layer"))
                {
                    ImGui::BeginChildFrame(8, ImVec2(2048, 840), 0);

                    ImVec2 p = ImGui::GetCursorScreenPos();

                    ImDrawList* draw_list = ImGui::GetWindowDrawList();
                    draw_list->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + 2048, p.y + 800), ImColor(26, 26, 26, 256), 0, ImDrawFlags_RoundCornersAll);

                    render(p, kb);

                    ImGui::EndChildFrame();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("SYM Layer"))
                {
                    ImGui::BeginChildFrame(8, ImVec2(2048, 840), 0);

                    ImVec2 p = ImGui::GetCursorScreenPos();

                    ImDrawList* draw_list = ImGui::GetWindowDrawList();
                    draw_list->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + 2048, p.y + 800), ImColor(26, 26, 26, 256), 0, ImDrawFlags_RoundCornersAll);

                    render(p, kb);

                    ImGui::EndChildFrame();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("RAISE Layer"))
                {
                    ImGui::BeginChildFrame(8, ImVec2(2048, 840), 0);

                    ImVec2 p = ImGui::GetCursorScreenPos();

                    ImDrawList* draw_list = ImGui::GetWindowDrawList();
                    draw_list->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + 2048, p.y + 800), ImColor(26, 26, 26, 256), 0, ImDrawFlags_RoundCornersAll);

                    render(p, kb);

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