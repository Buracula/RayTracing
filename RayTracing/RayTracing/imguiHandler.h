#pragma once

#include <d3d11.h>
#include <imgui.h>

class ImguiHandler
{
	ID3D11DeviceContext *mD3dDeviceContext;
	ID3D11Device *mD3dDevice;
	HWND mWindowHandle;

	ID3D11SamplerState *g_pFontSampler;
	ID3D11VertexShader *g_pVertexShader;
	ID3D11PixelShader *g_pPixelShader;
	ID3D11InputLayout *g_pInputLayout;
	ID3D11Buffer *g_pVertexConstantBuffer;
	ID3D11BlendState *g_pBlendState;
	ID3D11RasterizerState *g_pRasterizerState;
	ID3D11ShaderResourceView *g_pFontTextureView;
	ID3D11Buffer*            g_pVB;
	ID3D11Buffer*            g_pIB;
	int g_VertexBufferSize;
	int g_IndexBufferSize;

	INT64 g_Time;
	INT64 g_TicksPerSecond;


public:
	ImguiHandler(ID3D11DeviceContext *d3dDeviceContext, ID3D11Device *d3dDevice, HWND windowHandle);
	void Init();
	void StartNewFrame();
	void Render();
	void OnRender(ImDrawData* draw_data);

	bool rebuildRequested;
	int sphereCount;
	bool sphereOverlap;
	float minSphereRadiuses;
	float maxSphereRadiuses;
};

extern ImguiHandler *imguiHandler;
