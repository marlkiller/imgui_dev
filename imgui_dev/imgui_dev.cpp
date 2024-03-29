﻿

#include <iostream>
#include "common_imgui.h"
#include <tchar.h>
#include <dwmapi.h>
#include "tools.h"
#include "global.h"


// hide_out_window
static bool bind_out_window = false;
static ImColor color_red = ImColor(255, 0, 0, 255);
static ImColor color_green = ImColor(0, 255, 0, 255);
static ImColor color_blue = ImColor(0, 0, 255, 255);
static ImColor color_black = ImColor(0, 0, 0, 255);
static ImColor color_white = ImColor(255, 255, 255, 255);

enum MyItemColumnID
{
    MyItemColumnID_ID,
    MyItemColumnID_Name,
    MyItemColumnID_Action,
    MyItemColumnID_Quantity,
    MyItemColumnID_Description
};

struct MyItem
{
    int         ID;
    const char* Name;
    int         Quantity;
};

// Data
static ID3D11Device* g_pd3dDevice = NULL;
static ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
static IDXGISwapChain* g_pSwapChain = NULL;
static ID3D11RenderTargetView* g_mainRenderTargetView = NULL;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);



static bool refresh_draw_data = false;
static ImVector<MyItem> items;
static int table_data_len = 10;
DWORD WINAPI MakeTableDataThread(LPVOID pM)
{
    int MIN = 1;
    int MAX = table_data_len;
    //items.resize(1);
    while (true)
    {
        if (!refresh_draw_data)
        {
            Sleep(1000);
            continue;
        }
        // TODO get with drawIndex
        int id = (MIN + rand() % (MAX - MIN + 1));

        bool exist = false;
        ImVector<MyItem>::iterator it;
        for (it = items.begin(); it != items.end(); it++)
            if (it->ID == id)
            {
                it->Quantity = it->Quantity + 1;
                exist = true;
                continue;
            }
        if (!exist)
        {
            MyItem item;
            item.ID = id;
            item.Name = "name";
            item.Quantity = id % 3;
            items.push_back(item);
        }
    }
    return 0;
}
 
// Main code
int main(int, char**)
{
    CreateThread(NULL, 0, MakeTableDataThread, NULL, 0, NULL);
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("ImGui Example"), NULL };
    ::RegisterClassEx(&wc);

    RECT RectGame = { 0 };

    if (bind_out_window)
    {
        long ret_process = tools::findProcessbyName(_T("notepad"));
        printf("ret_process:%d", ret_process);

        global::hwndGame = FindWindow(NULL, TEXT("无标题 - 记事本"));
        tools::getGameRect(global::hwndGame, RectGame);

        global::hwndCurrent = ::CreateWindowExW(
            /*WS_EX_TOPMOST |*/ /*WS_EX_TRANSPARENT|*/ WS_EX_LAYERED,
            wc.lpszClassName,      // window class name
            _T("ImGui Example"),   // window caption
            WS_POPUP/*WS_OVERLAPPEDWINDOW*/, // window style, WS_POPUP can't show title
            RectGame.left, RectGame.top, RectGame.right - RectGame.left, RectGame.bottom - RectGame.top, // initial y size
            global::hwndGame, // parent window handle
            NULL, // window menu handle
            GetModuleHandle(NULL), // program instance handle
            NULL);
    }
    else {
        int screenwidth = GetSystemMetrics(SM_CXFULLSCREEN)/2;
        int screenheight = GetSystemMetrics(SM_CYFULLSCREEN)/2;
        global::hwndCurrent = ::CreateWindowExW(WS_EX_LAYERED , wc.lpszClassName, _T("ImGui Example"), WS_OVERLAPPEDWINDOW, 0, 0, screenwidth, screenheight, NULL, NULL, GetModuleHandle(NULL), NULL);
        tools::getGameRect(global::hwndCurrent, RectGame);

    }

    //关键色过滤
    //bAlpha, // 设置透明度，0表示完全透明，255表示不透明
    /*dwFlags参数可取以下值：
        LWA_ALPHA时：crKey参数无效，bAlpha参数有效；
        LWA_COLORKEY：窗体中的所有颜色为crKey的地方将变为透明，bAlpha参数无效。其常量值为1。
        LWA_ALPHA | LWA_COLORKEY：crKey的地方将变为全透明，而其它地方根据bAlpha参数确定透明度*/
    SetLayeredWindowAttributes(global::hwndCurrent, RGB(114,114,114), NULL, LWA_COLORKEY); // //设置颜色过滤,使用改关键色刷新屏幕后颜色被过滤实现透明

    //SetLayeredWindowAttributes(global::hwndCurrent, RGB(114, 114, 114), 255, LWA_ALPHA| LWA_COLORKEY); 

    //dwm透明特效, 搭配 LWA_ALPHA使用,进行透明后鼠标无法穿透透明部分
    //SetLayeredWindowAttributes(global::hwndCurrent, NULL, 255, LWA_ALPHA); // 不透明
    //DWM_BLURBEHIND bb = { 0 };
    //HRGN hRgn = CreateRectRgn(0, 0, -1, -1);
    //bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
    //bb.hRgnBlur = hRgn;
    //bb.fEnable = TRUE;
    //DwmEnableBlurBehindWindow(global::hwndCurrent, &bb);




    // Initialize Direct3D
    if (!CreateDeviceD3D(global::hwndCurrent))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    //::ShowWindow(global::hwndCurrent, SW_SHOW);
    /*MARGINS Margin = { -1, -1, -1, -1 };
    DwmExtendFrameIntoClientArea(global::hwndCurrent, &Margin);*/

    // Show the window
    ::ShowWindow(global::hwndCurrent, SW_SHOWDEFAULT);
    ::UpdateWindow(global::hwndCurrent);
    ::SetWindowPos(global::hwndCurrent, HWND_TOPMOST, RectGame.left, RectGame.top, RectGame.right - RectGame.left, RectGame.bottom - RectGame.top, SWP_SHOWWINDOW); // make win top


    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;




    ImFont* font1 = io.Fonts->AddFontFromMemoryTTF((void*)font_diy_data, font_diy_size, 16.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
    // cpu will heigh, will error if .ttc not exist , plz use memery font!AddFontFromMemaryTTF
    //ImFont* font1 = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyh.ttc", 16.0f,NULL,io.Fonts->GetGlyphRangesChineseFull());

    ImGuiStyle* style = &ImGui::GetStyle();

    // style 
    //style->FrameRounding = 12.0f;
    //style->GrabRounding = 12.0f;


    // Setup Dear ImGui style,will be cover the DIY color
    //ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();
    ImGui::StyleColorsLight();

    // style->Colors[ImGuiCol_ButtonHovered] = ImColor(255, 255, 255, 255);
    style->Colors[ImGuiCol_Text] = color_black;

    // color
    style->Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(global::hwndCurrent);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);


    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    //ImVec4 clear_color = ImVec4(0.00f,0.00f,0.00f,1.00f); //设置dx11屏幕刷新颜色 注意这里的颜色要和设置透明关键色设置一样
    //ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    ImVec4 clear_color = ImColor(114, 114, 114, 255).Value;
 

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        

        // height and width
        /*static int sub_win_height = 400;
        static int sub_win_width = 200;*/


        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 下个窗口位置居中
        //ImGui::SetNextWindowSize(ImVec2(200, 400));
        //ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x / 2 - (sub_win_width / 2), io.DisplaySize.y / 2 - (sub_win_height / 2)), ImGuiCond_FirstUseEver); // 去掉 ImGuiCond_FirstUseEver 后,窗口不能移动
        //ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x / 2 - (sub_win_width / 2), io.DisplaySize.y / 2 - (sub_win_height / 2))); 


        //设置窗口位置
        ImGui::SetNextWindowPos(ImVec2(0, 0), 0, ImVec2(0, 0));
        //设置窗口的大小
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y));
        //设置窗口为透明
        //ImGui::SetNextWindowBgAlpha(0); // 窗口透明的时候 字体渲染有问题
        //设置窗口的padding为0是图片控件充满窗口
        //ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        //设置窗口为无边框
        //ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);

        // remove the three example win
        // custom code

        static bool p_open = true; // monitor the window status
        static int active = 0; // monitor redio btn active
        static bool ck_v = false;

        static char* input_1 = NULL;
        static struct ExampleAppLog log;
        static RECT RectGame = { 0 };

        if (p_open)
        {
            // create the windows
            ImGui::Begin(u8"My Windows 窗口", &p_open); // chinese will mojibake if not set font
            ImGui::Text("Application average \n%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

            if (ImGui::Button("Button_1"))
            {
                log.AddLog("[%s] %s", "debug", "fuckyou\n");
            }

            ImGui::SameLine();// next node will same line
            ImGui::Button("Button_2");
            if (ImGui::Button("OPEN/CLOSE"))
            {
                log.AddLog("[%s] %s", "debug", "btn3 click event\n");
                p_open = false;
            }


            ImGui::RadioButton("RadioBtn_1", &active, 0); // warn: node name is identity 
            ImGui::RadioButton("RadioBtn_2", &active, 1);
            ImGui::RadioButton("RadioBtn_3", &active, 2);
            //printf("radio active : %d\n", active);
            ImGui::Checkbox("Refresh Draw Data", &refresh_draw_data);
            
            if (ImGui::Checkbox("To Log Window", &ck_v))
            {
                // checckbox_event
                log.AddLog("[%s] %s %d , %s %d\n", "debug", "check_box_1 val", ck_v, "radio_val", active);
            }
            if (ck_v) // checckbox val is true
            {
                style->FrameRounding = 12.0f;
                style->GrabRounding = 12.0f;


                ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
                ImGui::Begin("Example: Log", &ck_v);
                if (ImGui::SmallButton("[Debug] Add 5 entries"))
                {
                    static int counter = 0;
                    const char* categories[3] = { "info", "warn", "error" };
                    const char* words[] = { "Bumfuzzled", "Cattywampus", "Snickersnee", "Abibliophobia", "Absquatulate", "Nincompoop", "Pauciloquent" };
                    for (int n = 0; n < 5; n++)
                    {
                        const char* category = categories[counter % IM_ARRAYSIZE(categories)];
                        const char* word = words[counter % IM_ARRAYSIZE(words)];
                        log.AddLog("[%05d] [%s] Hello, current time is %.1f, here's a word: '%s'\n",
                            ImGui::GetFrameCount(), category, ImGui::GetTime(), word);
                        counter++;
                    }
                }
                ImGui::End();
                log.Draw("Example: Log", &ck_v);
            }
            else {

                style->FrameRounding = 0.0f;
                style->GrabRounding = 0.0f;
            }



            ImGui::Text("This is text");
            ImGui::BulletText("This is BulletText");
            ImGui::SameLine();
            common_imgui::HelpMarker("This is help makrer");

            static float f = 0; // if not statis , val will be reset
            ImGui::SliderFloat("This is sliderfloat", &f, 0, 1, "Process:%.3f%%");

            static int i = 0;
            ImGui::SliderInt("This is sliderint", &i, 0, 100, "Process:%d%%");


            static ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV;

            // Update item list if we changed the number of items
            static ImVector<int> selection;
            if (ImGui::BeginTable("table_advanced", 6, flags, ImVec2(0, 0), 0.0f))
            {
                ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 0.0f, MyItemColumnID_ID);
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 0.0f, MyItemColumnID_Name);
                ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, MyItemColumnID_Action);
                ImGui::TableSetupColumn("Quantity", ImGuiTableColumnFlags_NoSort, 0.0f, MyItemColumnID_Quantity);
                ImGui::TableSetupColumn("Description", (flags & ImGuiTableFlags_NoHostExtendX) ? 0 : ImGuiTableColumnFlags_WidthStretch, 0.0f, MyItemColumnID_Description);
                ImGui::TableSetupColumn("Hidden", ImGuiTableColumnFlags_DefaultHide | ImGuiTableColumnFlags_NoSort);
                ImGui::TableSetupScrollFreeze(1, 1);
                ImGui::TableHeadersRow();
                ImGui::PushButtonRepeat(true);
#if 1
                // Demonstrate using clipper for large vertical lists
                ImGuiListClipper clipper;
                clipper.Begin(items.Size);
                while (clipper.Step())
                {
                    for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
#else
                    {
                        for (int row_n = 0; row_n < items.Size; row_n++)
#endif
                        {
                            MyItem* item = &items[row_n];
                            //if (!filter.PassFilter(item->Name))
                            //    continue;

                            const bool item_is_selected = selection.contains(item->ID);
                            ImGui::PushID(item->ID);
                            ImGui::TableNextRow(ImGuiTableRowFlags_None, 0.0f);

                            // For the demo purpose we can select among different type of items submitted in the first column
                            ImGui::TableSetColumnIndex(0);
                            char label[32];
                            sprintf(label, "%04d", item->ID);
                            ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;
                            if (ImGui::Selectable(label, item_is_selected, selectable_flags, ImVec2(0, 0)))
                            {
                                selection.clear();
                                selection.push_back(item->ID);
                        }

                            if (ImGui::TableSetColumnIndex(1))
                                ImGui::Text(item->Name);

                            if (ImGui::TableSetColumnIndex(2))
                            {
                                if (ImGui::SmallButton("Chop")) { item->Quantity += 1; }
                                ImGui::SameLine();
                                if (ImGui::SmallButton("Eat")) { item->Quantity -= 1; }
                            }

                            if (ImGui::TableSetColumnIndex(3))
                                ImGui::Text("%d", item->Quantity);

                            if (ImGui::TableSetColumnIndex(4))
                                ImGui::Text("Lorem ipsum dolor sit amet");
                                

                            if (ImGui::TableSetColumnIndex(5))
                                ImGui::Text("");

                            ImGui::PopID();
                        }
                    }
                    ImGui::PopButtonRepeat();
                    ImGui::EndTable();
                }

            static char input_val[128] = "Hello, world!";
            if (ImGui::CollapsingHeader(u8"展开内容"))
            {
                ImGui::Text("this is text2");
                ImGui::InputText("this is label", input_val, IM_ARRAYSIZE(input_val), ImGuiInputTextFlags_CallbackEdit, Funcs::MyCallback);
            }

            
            

            //ImDrawList* drawList = ImGui::GetForegroundDrawList();
            // draw rect
            //drawList->AddRect(ImVec2(200, 200), ImVec2(300, 300), color_red);// closed when open is false

            //static ExampleAppLog my_log;

            ImGui::End();
            // ImGui::Button("btn4"); // btn will create in new window

            if (global::hwndGame)
            {
                tools::getGameRect(global::hwndGame, RectGame); // auto move windows
                MoveWindow(global::hwndCurrent, RectGame.left, RectGame.top, RectGame.right - RectGame.left, RectGame.bottom - RectGame.top, true);
            }

        }
        ImGui::EndFrame();

        //ImDrawList* drawList = ImGui::GetForegroundDrawList();
        //// draw 4 line
        //drawList->AddLine(ImVec2(100, 100), ImVec2(200, 100), color_red, 1.0f);
        //drawList->AddLine(ImVec2(200, 100), ImVec2(200, 200), color_red, 1.0f);
        //drawList->AddLine(ImVec2(100, 100), ImVec2(100, 200), color_red, 1.0f);
        //drawList->AddLine({ 100, 200 }, { 200, 200 }, color_red, 1.0f);

        //// draw text
        //drawList->AddText(ImVec2(100, 100), color_red, "this is text_begin");



        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // Present with vsync
        //g_pSwapChain->Present(0, 0); // Present without vsync
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(global::hwndCurrent);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    ::CloseHandle(global::hwndCurrent);
    ::CloseHandle(global::hwndGame);
    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
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
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
