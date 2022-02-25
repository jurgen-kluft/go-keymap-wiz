#include "libimgui/imgui.h"
#include "libimgui/imgui_internal.h"
#include "libimgui/imgui_impl_glfw.h"
#include "libimgui/imgui_impl_opengl3.h"

#include <stdio.h>
#include <math.h>           // sqrtf, powf, cosf, sinf, floorf, ceilf

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
		m_nob = false;
		m_index_in_keymap = 0;
		m_label = "Q";
		m_w = 80.0f;
		m_h = 80.0f;

		m_capcolor = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
		m_ledcolor = ImVec4(1.0f, 0.1f, 0.1f, 1.0f);
		m_txtcolor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	}
	// key shape (normal, L, ...)
	// nob

	bool m_nob; // home-key (e.g. the F or J key)

	int m_index_in_keymap;
	const char* m_label;

	float m_w;  // key width
	float m_h;  // key height

	ImVec4 m_capcolor;
	ImVec4 m_txtcolor;
	ImVec4 m_ledcolor;

};

struct col_t
{
	float m_x;
	float m_y;
	int  m_r;  // -45 degrees to 45 degrees (granularity is 1 degree)

	float m_w;  // key width
	float m_h;  // key height
	float m_sw; // key spacing width
	float m_sh;	// key spacing height

	int m_nb_keys;
	key_t* m_keys;
};

struct row_t
{
	float m_x;
	float m_y;
	int  m_r;  // -45 degrees to 45 degrees (granularity is 1 degree)

	float m_w;  // key width
	float m_h;  // key height
	float m_sw; // key spacing width
	float m_sh;	// key spacing height

	int m_nb_keys;
	key_t* m_keys;
};

struct keyboard_t
{
	keyboard_t()
	{
		m_colums = nullptr;
		m_rows = nullptr;
		m_scale = 1.0f;
		m_w = 80.f;
		m_h = 80.f;
		m_sw = 10.f;
		m_sh = 10.f;

		m_is_split_kb = true;
		m_has_underglow_rgb = true;
		m_has_per_key_rgb = false;
	}

	col_t** m_colums;
	row_t** m_rows;

	float m_scale;

	float m_w;  // key width
	float m_h;  // key height
	float m_sw; // key spacing width
	float m_sh;	// key spacing height

	bool m_is_split_kb;
	bool m_has_underglow_rgb;
	bool m_has_per_key_rgb;

};

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

static ImVec4 Darken(ImVec4 const& c, float p)
{
	ImVec4 r;
	r.x = c.x - (c.x * p);
	r.y = c.y - (c.y * p);
	r.z = c.z - (c.z * p);
	r.w = c.w - (c.w * p);
	return r;
}

ImVec2 operator-(const ImVec2& l, const ImVec2& r) { return{ l.x - r.x, l.y - r.y }; }
struct ImRotation
{
	ImRotation(ImDrawList* draw_list)
	{
		m_draw_list = draw_list;
		m_start = draw_list->VtxBuffer.Size;
	}

	ImVec2 Center()
	{
		ImVec2 l(FLT_MAX, FLT_MAX), u(-FLT_MAX, -FLT_MAX); // bounds

		const auto& buf = m_draw_list->VtxBuffer;
		for (int i = m_start; i < buf.Size; i++)
			l = ImMin(l, buf[i].pos), u = ImMax(u, buf[i].pos);

		return ImVec2((l.x+u.x)/2, (l.y+u.y)/2); // or use _ClipRectStack?
	}

	void Apply(float rad)
	{
		ImVec2 center = Center();

		float s=(float)sin(rad), c=(float)cos(rad);
		center = ImRotate(center, s, c) - center;

		auto& buf = m_draw_list->VtxBuffer;
		for (int i = m_start; i < buf.Size; i++)
			buf[i].pos = ImRotate(buf[i].pos, s, c) - center;
	}

	ImDrawList* m_draw_list;
	int m_start;
};

static void DrawKey(keyboard_t const& kb, float cx, float cy, float r, key_t const& key)
{
	const float thickness = 10.0f*kb.m_scale;
	const float kw = key.m_w * kb.m_scale;
	const float kh = key.m_h * kb.m_scale;
	const float rounding = kw / 5.0f;
	const float x = cx + 4.0f;
	const float y = cy + 4.0f;
	const float th = thickness;

	const ImU32 rkeycapcolor = ImColor(key.m_capcolor);
	ImVec4 dkeyledcolor(key.m_ledcolor);
	const ImU32 rkeyledcolor1 = ImColor(Darken(key.m_ledcolor, 0.2f));
	const ImU32 rkeyledcolor2 = ImColor(Darken(key.m_ledcolor, 0.1f));
	const ImU32 rkeyledcolor3 = ImColor(key.m_ledcolor);

	//draw_list->AddRect(ImVec2(x, y), ImVec2(x + sz, y + sz), rkeyledcolor, rounding, ImDrawFlags_RoundCornersAll, th/1.0f);
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImRotation rotation(draw_list);

	draw_list->AddRect(ImVec2(x, y), ImVec2(x + kw, y + kh), (rkeyledcolor1), rounding, ImDrawFlags_None, th/1.0f);
	draw_list->AddRect(ImVec2(x, y), ImVec2(x + kw, y + kh), (rkeyledcolor2), rounding, ImDrawFlags_None, th/2.0f);
	draw_list->AddRect(ImVec2(x, y), ImVec2(x + kw, y + kh), (rkeyledcolor3), rounding, ImDrawFlags_None, th/3.0f);
	draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + kw, y + kh), rkeycapcolor, rounding, ImDrawFlags_RoundCornersAll);

	rotation.Apply(r);
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
	//glfwWindowHint(GLFW_DECORATED, GL_FALSE);

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(2048, 720, "QMK Keymap Wizard", NULL, NULL);
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
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    // IM_ASSERT(font != NULL);

    // Our state
    bool   show_demo_window    = true;
    bool   show_another_window = false;
    ImVec4 clear_color         = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

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

			ImGui::SetNextWindowPos( ImVec2(0,0) );
			ImGui::Begin("Keyboard Wiz", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

			ImGui::BeginChildFrame(7, ImVec2(400, 200), 0);
			
            ImGui::Text("counter = %d", counter);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

			const float MIN_SCALE = 0.3f;
			const float MAX_SCALE = 2.0f;
			ImGui::DragFloat("global scale", &io.FontGlobalScale, 0.005f, MIN_SCALE, MAX_SCALE, "%.2f", ImGuiSliderFlags_AlwaysClamp); // Scale everything

			static key_t key;
			static keyboard_t kb;
			kb.m_scale = io.FontGlobalScale;

			static ImGuiColorEditFlags alpha_flags = 0;
			static ImVec4 keycapcolor = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
			ImGui::ColorEdit4("key cap color", (float*)&key.m_capcolor, ImGuiColorEditFlags_AlphaBar | alpha_flags);

			static ImVec4 keyledcolor = ImVec4(1.0f, 0.1f, 0.1f, 1.0f);
			ImGui::ColorEdit4("key led color", (float*)&key.m_ledcolor, ImGuiColorEditFlags_AlphaBar | alpha_flags);

			ImGui::EndChildFrame();

			// Tip: If you do a lot of custom rendering, you probably want to use your own geometrical types and benefit of
			// overloaded operators, etc. Define IM_VEC2_CLASS_EXTRA in imconfig.h to create implicit conversions between your
			// types and ImVec2/ImVec4. Dear ImGui defines overloaded operators but they are internal to imgui.cpp and not
			// exposed outside (to avoid messing with your types) In this example we are not using the maths operators!

			if (ImGui::BeginTabBar("##TabBar"))
			{
				if (ImGui::BeginTabItem("QWERTY Layer"))
				{
					ImGui::BeginChildFrame(8, ImVec2(2048, 640), 0);

					ImGui::PushItemWidth(-ImGui::GetFontSize() * 15);
					ImDrawList* draw_list = ImGui::GetWindowDrawList();

					ImVec2 p = ImGui::GetCursorScreenPos();

					// Draw a bunch of primitives
					float ks = io.FontGlobalScale;
					float kw = 80.0f*ks;
					float kh = 80.0f*ks;
					float thickness = 10.0f*ks;
					ImVec4 colf = ImVec4(1.0f, 1.0f, 0.4f, 1.0f);
					const ImU32 col = ImColor(colf);
					const float sw = 10.0f;
					const float sh = 10.0f;
					const ImDrawFlags corners_tl_br = ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersBottomRight;
					const float rounding = kw / 5.0f;
					float th = thickness*2;

					float x = p.x + 4.0f;
					float y = p.y + 4.0f;

					draw_list->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + ((kw+sw) * 22), p.y + ((kh+sh)*8)), ImColor(26,26,26,256), 0, ImDrawFlags_RoundCornersAll);

					y = p.y + 4.0f;
					for (int row = 0; row < 3; ++row)
					{
						x = p.x + 4.0f;

						DrawKey(kb, x + kw / 2, y + kh / 2, 0, key);
						x += kw + sw;
						DrawKey(kb, x + kw / 2, y + kh / 2, 0, key);
						x += kw + sw;
						DrawKey(kb, x + kw / 2, y + kh / 2, 0, key);
						x += kw + sw;
						DrawKey(kb, x + kw / 2, y + kh / 2, 0, key);
						x += kw + sw;
						DrawKey(kb, x + kw / 2, y + kh / 2, 0, key);
						x += kw + sw;
						DrawKey(kb, x + kw / 2, y + kh / 2, 0, key);

						x += kw + sw;
						x += kw + sw;

						x += kw + sw;

						x += kw + sw;
						x += kw + sw;

						x += kw + sw;
						DrawKey(kb, x + kw / 2, y + kh / 2, 0, key);
						x += kw + sw;
						DrawKey(kb, x + kw / 2, y + kh / 2, 0, key);
						x += kw + sw;
						DrawKey(kb, x + kw / 2, y + kh / 2, 0, key);
						x += kw + sw;
						DrawKey(kb, x + kw / 2, y + kh / 2, 0, key);
						x += kw + sw;
						DrawKey(kb, x + kw / 2, y + kh / 2, 0, key);
						x += kw + sw;
						DrawKey(kb, x + kw / 2, y + kh / 2, 0, key);

						y += kh + sh;
					}

					ImGui::EndChildFrame();
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("SYM Layer"))
				{
					ImGui::BeginChildFrame(8, ImVec2(2048, 640), 0);
					
					// Render the sym layer

					ImGui::EndChildFrame();
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("RAISE Layer"))
				{
					ImGui::BeginChildFrame(8, ImVec2(2048, 640), 0);
					
					// Render the raise layer

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
		if (winSize.y < 720)
			winSize.y = 720;

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
		//glfwSetWindowPos( window, ((int)winSize.x) / 2, ((int)winSize.y) / 2 );
		glfwSetWindowSize( window, (int)winSize.x, (int)winSize.y ); // Resize
	}

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}