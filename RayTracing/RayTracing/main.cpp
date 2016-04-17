#include <windows.h>
#include <d3d11.h>
#include <glm/glm.hpp>
#include <imgui.h>

#include "imguiHandler.h"
#include "RayTracer.h"

IMGUI_API LRESULT ImGui_ImplDX11_WndProcHandler(HWND, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ImGuiIO& io = ImGui::GetIO();
	switch (msg)
	{
	case WM_LBUTTONDOWN:
		io.MouseDown[0] = true;
		return true;
	case WM_LBUTTONUP:
		io.MouseDown[0] = false;
		return true;
	case WM_RBUTTONDOWN:
		io.MouseDown[1] = true;
		return true;
	case WM_RBUTTONUP:
		io.MouseDown[1] = false;
		return true;
	case WM_MBUTTONDOWN:
		io.MouseDown[2] = true;
		return true;
	case WM_MBUTTONUP:
		io.MouseDown[2] = false;
		return true;
	case WM_MOUSEWHEEL:
		io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
		return true;
	case WM_MOUSEMOVE:
		io.MousePos.x = (signed short)(lParam);
		io.MousePos.y = (signed short)(lParam >> 16);
		return true;
	case WM_KEYDOWN:
		if (wParam < 256)
			io.KeysDown[wParam] = 1;
		return true;
	case WM_KEYUP:
		if (wParam < 256)
			io.KeysDown[wParam] = 0;
		return true;
	case WM_CHAR:
		// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
		if (wParam > 0 && wParam < 0x10000)
			io.AddInputCharacter((unsigned short)wParam);
		return true;
	}
	return 0;
}

LRESULT CALLBACK windowPrecedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplDX11_WndProcHandler(hwnd, msg, wParam, lParam))
		return true;

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	//////////////////////////////////////////////////////////////////////////
	//Create window
	//////////////////////////////////////////////////////////////////////////
	int screenWidth = 1280;
	int screenHeight = 760;

	WNDCLASS windowClass;
	ZeroMemory(&windowClass, sizeof(WNDCLASS));

	windowClass.hInstance = hInstance;
	windowClass.lpszClassName = "windowClass";
	windowClass.lpfnWndProc = windowPrecedure;
	windowClass.style = CS_HREDRAW | CS_VREDRAW;

	RegisterClass(&windowClass);

	RECT rect = { 0,0, screenWidth, screenHeight };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);

	HWND windowHandle = CreateWindow("windowClass", "Ray Tracing", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 
		rect.right - rect.left, rect.bottom - rect.top, 0, 0, hInstance, nullptr);
	ShowWindow(windowHandle, SW_SHOW);

	//////////////////////////////////////////////////////////////////////////
	//Init directx
	//////////////////////////////////////////////////////////////////////////

	ID3D11Device *d3dDevice = nullptr;
	ID3D11DeviceContext *d3dDeviceContext = nullptr;
	IDXGISwapChain *d3dSwapChain = nullptr;
#ifdef _DEBUG
	unsigned int deviceFlag = D3D11_CREATE_DEVICE_DEBUG;
#else
	unsigned int deviceFlag = 0;
#endif

	D3D_FEATURE_LEVEL levels = D3D_FEATURE_LEVEL_11_0;
	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
	scd.BufferCount = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	scd.BufferDesc.Height = screenHeight;
	scd.BufferDesc.Width = screenWidth;
	scd.BufferDesc.RefreshRate.Numerator = 60;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = windowHandle;
	scd.SampleDesc.Count = 1;
	scd.Windowed = TRUE;
	D3D_FEATURE_LEVEL outLevel;
	D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, deviceFlag, &levels, 1, D3D11_SDK_VERSION,
		&scd, &d3dSwapChain, &d3dDevice, &outLevel, &d3dDeviceContext);

	ID3D11Texture2D *screenTexture;
	ID3D11RenderTargetView *screenTextureRTV;
	d3dSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&screenTexture);

	d3dDevice->CreateRenderTargetView(screenTexture, nullptr, &screenTextureRTV);
	screenTexture->Release();
	screenTexture = nullptr;

	d3dDeviceContext->OMSetRenderTargets(1, &screenTextureRTV, nullptr);
	//////////////////////////////////////////////////////////////////////////
	//UI
	//////////////////////////////////////////////////////////////////////////

	imguiHandler = new ImguiHandler(d3dDeviceContext, d3dDevice, windowHandle);
	imguiHandler->Init();

	RayTracer tracer;
	
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}
		imguiHandler->StartNewFrame();
		imguiHandler->Render();

		d3dSwapChain->Present(0, 0);
	}

	return 0;
}