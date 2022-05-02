#pragma once



#include "imgui\imconfig.h"
#include "imgui\imgui.h"
#include "imgui\imgui_impl_dx11.h"
#include "imgui\imgui_impl_win32.h"
#include "imgui\imgui_internal.h"
#include "imgui\imstb_rectpack.h"
#include "imgui\imstb_textedit.h"
#include "imgui\imstb_truetype.h"


#include <d3d11.h>
#pragma comment(lib,"d3d11.lib")


namespace imgui_common
{
	void ShowExampleAppLog(bool* p_open);
	void print_log();


}