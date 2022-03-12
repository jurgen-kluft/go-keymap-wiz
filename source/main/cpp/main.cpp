#include "xbase/x_base.h"
#include "xbase/x_memory.h"

#include "qmk-keymap-wiz/keyboard_data.h"
#include "qmk-keymap-wiz/keyboard_render.h"

#include "libimgui/imgui.h"
#include "libimgui/imgui_internal.h"
#include "libimgui/imgui_impl_glfw.h"
#include "libimgui/imgui_impl_opengl3.h"

#include <stdio.h>
#include <math.h> // sqrtf, powf, cosf, sinf, floorf, ceilf

#define GL_SILENCE_DEPRECATION
#include "libglfw/glfw3.h" // Will drag in system OpenGL headers

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

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
    keyboard_loadfonts();

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    xcore::keyboards_t const* keebs = nullptr;
    xcore::init_keyboards();
    xcore::load_keyboards(keebs);
    xcore::keyboard_t const* kb = &keebs->m_keyboards[0];

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        if (xcore::reload_keyboards(keebs))
        {
            kb = &keebs->m_keyboards[0];
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
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::Begin("Keyboard Wiz", nullptr,
                         ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

            ImGui::BeginChildFrame(ImGui::GetID("Settings"), ImVec2(320, 200), ImGuiWindowFlags_NoMove);

            // When clicking a key we would like a menu to popup
            // The menu should be able to provide to the user:
            //   - change the key's keycode
            //   - change the key's modifiers (shift, ctrl, alt, etc.)
            //   - change the key's cap color
            //   - change the key's led color

            // Adding/Removing layers
            //   - add a new layer (see if there was a layer with the same name that is marked as deleted')
            //   - remove a layer (when removing we should just mark it as 'deleted')

            if (ImGui::Button("Key Pressed"))
                ImGui::OpenPopup("Key Properties");

            if (ImGui::BeginPopupModal("Key Properties", NULL, ImGuiWindowFlags_MenuBar))
            {
                if (ImGui::BeginMenuBar())
                {
                    if (ImGui::BeginMenu("File"))
                    {
                        if (ImGui::MenuItem("Some menu item")) {}
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }
                ImGui::Text("Hello from Stacked The First\nUsing style.Colors[ImGuiCol_ModalWindowDimBg] behind it.");

                // Testing behavior of widgets stacking their own regular popups over the modal.
                static int item = 1;
                static float color[4] = { 0.4f, 0.7f, 0.0f, 0.5f };
                ImGui::Combo("Combo", &item, "aaaa\0bbbb\0cccc\0dddd\0eeee\0\0");
                ImGui::ColorEdit4("color", color);

                if (ImGui::Button("Close"))
                    ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
            }

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            const float MIN_SCALE = 0.7f;
            const float MAX_SCALE = 1.5f;
            ImGui::DragFloat("global scale", &io.FontGlobalScale, 0.005f, MIN_SCALE, MAX_SCALE, "%.2f", ImGuiSliderFlags_AlwaysClamp); // Scale everything

            ImGui::EndChildFrame();

            static ImVector<int> active_tabs;
            static int           next_tab_id = 0;
            if (next_tab_id == 0) // Initialize with some default tabs
            {
                for (int i = 0; i < 3; i++)
                    active_tabs.push_back(next_tab_id++);
            }

            static bool             show_leading_button  = true;
            static bool             show_trailing_button = true;
            static ImGuiTabBarFlags tab_bar_flags        = ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_FittingPolicyResizeDown;

            ImVec2 frameSize = ImVec2(2048, 840);
            ImGui::BeginChildFrame(ImGui::GetID("keyboard_area"), frameSize);
            if (ImGui::BeginTabBar("Layers", tab_bar_flags))
            {
                ImColor tabkgrndcolor(10, 10, 10, 256);

                if (ImGui::TabItemButton("?", ImGuiTabItemFlags_Leading | ImGuiTabItemFlags_NoTooltip))
                    ImGui::OpenPopup("MyHelpMenu");
                if (ImGui::BeginPopup("MyHelpMenu"))
                {
                    ImGui::Selectable("Hello!");
                    ImGui::EndPopup();
                }

                if (ImGui::TabItemButton("+", ImGuiTabItemFlags_Trailing | ImGuiTabItemFlags_NoTooltip))
                    active_tabs.push_back(next_tab_id++); // Add new tab

                // Submit our regular tabs
                for (int n = 0; n < active_tabs.Size; n++)
                {
                    char name[16];
                    snprintf(name, IM_ARRAYSIZE(name), "%04d", active_tabs[n]);
                    if (ImGui::BeginTabItem(name))
                    {
                        ImVec2 p = ImGui::GetCursorScreenPos();

                        ImDrawList* draw_list = ImGui::GetWindowDrawList();
                        draw_list->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + frameSize.x, p.y + frameSize.y), tabkgrndcolor, 0, ImDrawFlags_RoundCornersAll);

                        keyboard_render(kb, p.x, p.y, io.MousePos.x, io.MousePos.y, io.FontGlobalScale);

                        ImGui::EndTabItem();
                    }
                }
                ImGui::EndTabBar();
            }
            ImGui::EndChildFrame();
            winSize = ImGui::GetWindowSize();

            ImGui::ShowDemoWindow();

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