// Dear ImGui: standalone example application for DirectX 9

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include <d3d9.h>
#include <tchar.h>
#include <imgui_internal.h>
#include "Sfpro.h"

// Data
static LPDIRECT3D9              g_pD3D = nullptr;
static LPDIRECT3DDEVICE9        g_pd3dDevice = nullptr;
static bool                     g_DeviceLost = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//added
bool GlowingCheckbox(const char* label, bool* v)
{
    // Get the current window's draw list
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Get the position before rendering the checkbox
    ImVec2 pos = ImGui::GetCursorScreenPos();

    // Apply rounding to the checkbox
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);

    // Render the checkbox
    ImGui::Checkbox(label, v);

    // Revert style changes
    ImGui::PopStyleVar();

    // Get the size of the checkbox (size of the last item)
    ImVec2 size = ImGui::GetItemRectSize();

    // Get the rectangle of the checkbox (excluding the label)
    float square_sz = ImGui::GetFrameHeight();
    ImVec2 checkbox_pos = pos;
    ImVec2 checkbox_size = ImVec2(square_sz, square_sz);
    ImRect checkbox_rect(checkbox_pos, ImVec2(checkbox_pos.x + checkbox_size.x, checkbox_pos.y + checkbox_size.y));

    // Only draw the glow if the checkbox is checked
    if (*v)
    {
        // Glow parameters
        int glow_layers = 5;
        float max_glow_thickness = 2.5f;
        ImVec4 glow_color = ImVec4(0.2f, 0.7f, 1.0f, 0.1f);

        // Draw glow effect
        for (int i = 0; i < glow_layers; ++i)
        {
            float progress = (float)i / (float)(glow_layers - 1);
            float thickness = progress * max_glow_thickness;
            ImVec4 layer_color = glow_color;
            layer_color.w *= (1.0f - progress); // Fade out outer layers

            ImRect expanded_rect = ImRect(
                checkbox_rect.Min.x - thickness,
                checkbox_rect.Min.y - thickness,
                checkbox_rect.Max.x + thickness,
                checkbox_rect.Max.y + thickness
            );

            draw_list->AddRectFilled(
                expanded_rect.Min,
                expanded_rect.Max,
                ImGui::ColorConvertFloat4ToU32(layer_color),
                5.0f // Rounding
            );
        }
    }

    return *v;
}



//added
bool GlowingToggleCheckbox(const char* label, bool* v)
{
    // Get the current window's draw list
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Get the position before rendering the checkbox
    ImVec2 pos = ImGui::GetCursorScreenPos();

    // Apply rounding to the checkbox
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);

    // Invisible button for interaction (toggle behavior)
    float height = ImGui::GetFrameHeight() - 1.0f;
    float width = height * 1.55f;
    float radius = height * 0.50f;

    ImGui::InvisibleButton(label, ImVec2(width, height));

    // Handle state change
    if (ImGui::IsItemClicked())
        *v = !(*v);

    // Revert style changes
    ImGui::PopStyleVar();

    // Get the size of the checkbox (size of the last item)
    ImVec2 checkbox_size = ImVec2(width, height);
    ImRect checkbox_rect(pos, ImVec2(pos.x + checkbox_size.x, pos.y + checkbox_size.y));

    // Render background based on state
    ImU32 col_bg = *v ? IM_COL32(62, 157, 251, 255) : IM_COL32(19, 24, 30, 255); // Green if on, red if off
    draw_list->AddRectFilled(checkbox_rect.Min, checkbox_rect.Max, col_bg, radius);



    // Only draw the glow if the checkbox is checked
    if (*v)
    {
        // Glow parameters
        int glow_layers = 10;
        float max_glow_thickness = 3.f;
        ImVec4 glow_color = ImVec4(0.2f, 0.7f, 1.0f, 0.1f);

        // Draw glow effect
        for (int i = 0; i < glow_layers; ++i)
        {
            float progress = (float)i / (float)(glow_layers - 1);
            float thickness = progress * max_glow_thickness;
            ImVec4 layer_color = glow_color;
            layer_color.w *= (1.0f - progress); // Fade out outer layers

            ImRect expanded_rect = ImRect(
                checkbox_rect.Min.x - thickness,
                checkbox_rect.Min.y - thickness,
                checkbox_rect.Max.x + thickness,
                checkbox_rect.Max.y + thickness
            );

            draw_list->AddRectFilled(
                expanded_rect.Min,
                expanded_rect.Max,
                ImGui::ColorConvertFloat4ToU32(layer_color),
                radius // Rounding
            );
        }
    }

    // Render circle for toggle
    ImU32 col_circle = IM_COL32(255, 255, 255, 255); // White circle
    float t = *v ? 1.0f : 0.0f;
    ImVec2 circle_pos = ImVec2(pos.x + radius + t * (width - radius * 2.0f), pos.y + radius);
    draw_list->AddCircleFilled(circle_pos, radius - 1.5f, col_circle);

    // Render the label next to the toggle
    ImGui::SameLine();
    ImGui::Text(label);

    return *v;
}

//added
void FadingButton(const char* buttonLabel, float fadeSpeed = 2.0f)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    ImVec2 p = window->DC.CursorPos;
    ImVec2 labelSize = ImGui::CalcTextSize(buttonLabel);
    ImVec2 padding = ImVec2(20, 10); 

 
    ImVec2 buttonSize = ImVec2(labelSize.x + padding.x, labelSize.y + padding.y);
    ImRect buttonRect(p, ImVec2(p.x + buttonSize.x, p.y + buttonSize.y));

    
    bool isHovered = ImGui::IsMouseHoveringRect(buttonRect.Min, buttonRect.Max);

    // Static variable to manage fade alpha state across frames
    static float fadeAlpha = 0.0f;

    // Update fadeAlpha based on hover state
    if (isHovered)
    {
        fadeAlpha = ImMin(fadeAlpha + fadeSpeed * ImGui::GetIO().DeltaTime, 1.0f);
    }
    else
    {
        fadeAlpha = ImMax(fadeAlpha - fadeSpeed * ImGui::GetIO().DeltaTime, 0.0f);
    }

    // Set button style and draw the button
    ImVec4 buttonColor = ImVec4(0.4f, 0.7f, 1.0f, fadeAlpha); // Light blue color with fading
    ImVec4 buttonHoveredColor = ImVec4(0.5f, 0.8f, 1.0f, fadeAlpha);
    ImVec4 buttonActiveColor = ImVec4(0.3f, 0.6f, 1.0f, fadeAlpha);

    ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonHoveredColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, buttonActiveColor);

    
    if (ImGui::Button(buttonLabel))
    {
        
        ImGui::Text("Button clicked!");
    }

    ImGui::PopStyleColor(3); // Pop the three style colors we pushed
}

// Main code
int main(int, char**)
{
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX9 Example", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);

    //added
    ImFontConfig font_cfg;
    font_cfg.FontDataOwnedByAtlas = false;
    io.Fonts->AddFontFromMemoryTTF(SFP, SFsize, 16.0f, &font_cfg);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


    bool toggle_state = false; //added
    bool my_value; //added
    float my_value1 = 3; //added


    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Handle lost D3D9 device
        if (g_DeviceLost)
        {
            HRESULT hr = g_pd3dDevice->TestCooperativeLevel();
            if (hr == D3DERR_DEVICELOST)
            {
                ::Sleep(10);
                continue;
            }
            if (hr == D3DERR_DEVICENOTRESET)
                ResetDevice();
            g_DeviceLost = false;
        }

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            g_d3dpp.BackBufferWidth = g_ResizeWidth;
            g_d3dpp.BackBufferHeight = g_ResizeHeight;
            g_ResizeWidth = g_ResizeHeight = 0;
            ResetDevice();
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state

            //added
            if (GlowingToggleCheckbox("Test Glow Enable Feature", &toggle_state)) { 
                // Handle the state change like a checkbox
                if (toggle_state) {
                    ImGui::Text("Feature is Enabled");
                }
                else {
                    ImGui::Text("Feature is Disabled");
                }
            }

            //added
            GlowingCheckbox("Test Glowing Checkbox", &my_value);

            //added
            FadingButton("Test Fading Button");
            //added
            ImGui::SliderFloatGlow("Test Glowing Slider", &my_value1, 0, 10);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
          

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::EndFrame();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x*clear_color.w*255.0f), (int)(clear_color.y*clear_color.w*255.0f), (int)(clear_color.z*clear_color.w*255.0f), (int)(clear_color.w*255.0f));
        g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }
        HRESULT result = g_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr);
        if (result == D3DERR_DEVICELOST)
            g_DeviceLost = true;
    }

    // Cleanup
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
        return false;

    // Create the D3DDevice
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
    //g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;

    return true;
}

void CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = nullptr; }
}

void ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
