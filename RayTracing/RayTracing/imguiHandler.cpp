#include "imguiHandler.h"
#include <d3dcompiler.h>
#include <string>

struct VERTEX_CONSTANT_BUFFER
{
	float        mvp[4][4];
};


void ImGui_render_callback(ImDrawData* draw_data)
{
	imguiHandler->OnRender(draw_data);
}

ImguiHandler::ImguiHandler(ID3D11DeviceContext *d3dDeviceContext, ID3D11Device *d3dDevice, HWND windowHandle)
{
	mD3dDevice = d3dDevice;
	mD3dDeviceContext = d3dDeviceContext;
	mWindowHandle = windowHandle;
	g_VertexBufferSize = 5000;
	g_IndexBufferSize = 10000;
	g_pVB = NULL;
	g_pIB = NULL;
	rebuildRequested = true;
	useGpu = true;
	showOctree = false;
	light1 = true;
	light2 = false;
	light3 = false;
	sphereCount = 4;
	sphereOverlap = true;
	minSphereRadiuses = 2;
	maxSphereRadiuses = 2;
	rayTracingTime = 0;

#ifdef BAHAR
	sphereOverlap = false;
	minSphereRadiuses = 1;
	maxSphereRadiuses = 5;
#endif

	sphereNum = 4;
	sphereNumText[0] = '4';
	sphereNumText[1] = '\0';
}

void ImguiHandler::Init()
{
	ImGuiIO& io = ImGui::GetIO();
	io.KeyMap[ImGuiKey_Tab] = VK_TAB;                       // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array that we will update during the application lifetime.
	io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
	io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
	io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
	io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
	io.KeyMap[ImGuiKey_Home] = VK_HOME;
	io.KeyMap[ImGuiKey_End] = VK_END;
	io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
	io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
	io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
	io.KeyMap[ImGuiKey_A] = 'A';
	io.KeyMap[ImGuiKey_C] = 'C';
	io.KeyMap[ImGuiKey_V] = 'V';
	io.KeyMap[ImGuiKey_X] = 'X';
	io.KeyMap[ImGuiKey_Y] = 'Y';
	io.KeyMap[ImGuiKey_Z] = 'Z';

	io.RenderDrawListsFn = ImGui_render_callback;  // Alternatively you can set this to NULL and call ImGui::GetDrawData() after ImGui::Render() to get the same ImDrawData pointer.
	io.ImeWindowHandle = mWindowHandle;
	// Create the vertex shader
	{
		static const char* vertexShader =
			"cbuffer vertexBuffer : register(b0) \
            {\
            float4x4 ProjectionMatrix; \
            };\
            struct VS_INPUT\
            {\
            float2 pos : POSITION;\
            float4 col : COLOR0;\
            float2 uv  : TEXCOORD0;\
            };\
            \
            struct PS_INPUT\
            {\
            float4 pos : SV_POSITION;\
            float4 col : COLOR0;\
            float2 uv  : TEXCOORD0;\
            };\
            \
            PS_INPUT main(VS_INPUT input)\
            {\
            PS_INPUT output;\
            output.pos = mul( ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));\
            output.col = input.col;\
            output.uv  = input.uv;\
            return output;\
            }";
		ID3DBlob *shaderBlob = nullptr;
		D3DCompile(vertexShader, strlen(vertexShader), NULL, NULL, NULL, "main", "vs_4_0", 0, 0, &shaderBlob, NULL);
		mD3dDevice->CreateVertexShader((DWORD*)shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL, &g_pVertexShader);

		// Create the input layout
		D3D11_INPUT_ELEMENT_DESC local_layout[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (size_t)(&((ImDrawVert*)0)->pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (size_t)(&((ImDrawVert*)0)->uv),  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, (size_t)(&((ImDrawVert*)0)->col), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		mD3dDevice->CreateInputLayout(local_layout, 3, shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), &g_pInputLayout);

		// Create the constant buffer
		{
			D3D11_BUFFER_DESC desc;
			desc.ByteWidth = sizeof(VERTEX_CONSTANT_BUFFER);
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			mD3dDevice->CreateBuffer(&desc, NULL, &g_pVertexConstantBuffer);
		}
		shaderBlob->Release();
	}

	// Create the pixel shader
	{
		static const char* pixelShader =
			"struct PS_INPUT\
            {\
            float4 pos : SV_POSITION;\
            float4 col : COLOR0;\
            float2 uv  : TEXCOORD0;\
            };\
            sampler sampler0;\
            Texture2D texture0;\
            \
            float4 main(PS_INPUT input) : SV_Target\
            {\
            float4 out_col = input.col * texture0.Sample(sampler0, input.uv); \
            return out_col; \
            }";
		ID3DBlob *shaderBlob;
		D3DCompile(pixelShader, strlen(pixelShader), NULL, NULL, NULL, "main", "ps_4_0", 0, 0, &shaderBlob, NULL);
		mD3dDevice->CreatePixelShader((DWORD*)shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL, &g_pPixelShader);
		shaderBlob->Release();
	}

	// Create the blending setup
	{
		D3D11_BLEND_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.AlphaToCoverageEnable = false;
		desc.RenderTarget[0].BlendEnable = true;
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		mD3dDevice->CreateBlendState(&desc, &g_pBlendState);
	}

	// Create the rasterizer state
	{
		D3D11_RASTERIZER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.FillMode = D3D11_FILL_SOLID;
		desc.CullMode = D3D11_CULL_NONE;
		desc.ScissorEnable = true;
		desc.DepthClipEnable = true;
		mD3dDevice->CreateRasterizerState(&desc, &g_pRasterizerState);
	}

	unsigned char* pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	// Upload texture to graphics system
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;

		ID3D11Texture2D *pTexture = NULL;
		D3D11_SUBRESOURCE_DATA subResource;
		subResource.pSysMem = pixels;
		subResource.SysMemPitch = desc.Width * 4;
		subResource.SysMemSlicePitch = 0;
		mD3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);

		// Create texture view
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = desc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		mD3dDevice->CreateShaderResourceView(pTexture, &srvDesc, &g_pFontTextureView);
		pTexture->Release();
	}

	// Store our identifier
	io.Fonts->TexID = (void *)g_pFontTextureView;

	// Create texture sampler
	{
		D3D11_SAMPLER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.MipLODBias = 0.f;
		desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		desc.MinLOD = 0.f;
		desc.MaxLOD = 0.f;
		mD3dDevice->CreateSamplerState(&desc, &g_pFontSampler);
	}
}

void ImguiHandler::StartNewFrame()
{
	QueryPerformanceFrequency((LARGE_INTEGER *)&g_TicksPerSecond);
	QueryPerformanceCounter((LARGE_INTEGER *)&g_Time);

	ImGuiIO& io = ImGui::GetIO();

	// Setup display size (every frame to accommodate for window resizing)
	RECT rect;
	GetClientRect(mWindowHandle, &rect);
	io.DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));

	// Setup time step
	INT64 current_time;
	QueryPerformanceCounter((LARGE_INTEGER *)&current_time);
	io.DeltaTime = (float)(current_time - g_Time) / g_TicksPerSecond;
	g_Time = current_time;

	// Read keyboard modifiers inputs
	io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
	io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
	io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
	io.KeySuper = false;
	// io.KeysDown : filled by WM_KEYDOWN/WM_KEYUP events
	// io.MousePos : filled by WM_MOUSEMOVE events
	// io.MouseDown : filled by WM_*BUTTON* events
	// io.MouseWheel : filled by WM_MOUSEWHEEL events
	bool show_test_window = true;
	// Hide OS mouse cursor if ImGui is drawing it
	SetCursor(io.MouseDrawCursor ? NULL : LoadCursor(NULL, IDC_ARROW));
	ImVec4 clear_col = ImColor(114, 144, 154);
	// Start the frame
	ImGui::NewFrame();
}

void ImguiHandler::Render()
{
#ifdef BAHAR
	Render2();
#else

	bool show_test_window = true;
	     // Normally user code doesn't need/want to call it because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
	//ImGui::ShowTestWindow(&show_test_window);
	ImGui::Begin("Settings");
	ImGui::SetWindowPos(ImVec2(850, 20), ImGuiSetCond_FirstUseEver);
	ImGui::SetWindowSize(ImVec2(400, 230), ImGuiSetCond_FirstUseEver);
	ImGui::SliderFloat("Min Sphere Radius", &minSphereRadiuses, 0.5f, 10.0f);
	ImGui::SliderFloat("Max Sphere Radius", &maxSphereRadiuses, 0.5f, 10.0f);
	ImGui::SliderInt("Sphere Count", &sphereCount, 1, 200);
	ImGui::Checkbox("Spheres can overlap", &sphereOverlap);
	ImGui::Checkbox("Use GPU", &useGpu);
	ImGui::Checkbox("Show Octree", &showOctree);
	ImGui::Checkbox("Enable Light 1", &light1);
	ImGui::Checkbox("Enable Light 2", &light2);
	ImGui::Checkbox("Enable Light 3", &light3);

	std::string rayTracingTimeText = std::to_string(rayTracingTime);
	ImGui::LabelText(rayTracingTimeText.c_str(), "RayTrace Time(ms):");

	if (ImGui::Button("Update"))
	{
		rebuildRequested = true;
	}
	ImGui::End();

	ImGui::Render();
#endif
}


void ImguiHandler::Render2()
{
	bool show_test_window = true;
	// Normally user code doesn't need/want to call it because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
	//ImGui::ShowTestWindow(&show_test_window);
	ImGui::Begin("Sphere Features");
	ImGui::SetWindowPos(ImVec2(10, 5), ImGuiSetCond_FirstUseEver);
	ImGui::SetWindowSize(ImVec2(350, 140), ImGuiSetCond_FirstUseEver);
	ImGui::PushItemWidth(100);
	ImGui::InputText("Sphere Number", sphereNumText, sizeof(sphereNumText));
	ImGui::PushItemWidth(200);
	ImGui::SliderFloat("Min Sphere Radius", &minSphereRadiuses, 0.5f, 10.0f, "%.1f");
	ImGui::SliderFloat("Max Sphere Radius", &maxSphereRadiuses, 0.5f, 10.0f, "%.1f");
	//ImGui::SliderInt("Sphere Count", &sphereCount, 1, 200);
	//ImGui::Checkbox("Spheres can overlap", &sphereOverlap);
	if (ImGui::Button("Ray Trace!"))
	{
		sphereNum = atoi(sphereNumText);
		rebuildRequested = true;
	}
	std::string rayTracingTimeText = std::to_string(rayTracingTime);
	ImGui::LabelText(rayTracingTimeText.c_str(), "Time in ms:");


	ImGui::End();

	ImGui::Render();
}

void ImguiHandler::OnRender(ImDrawData* draw_data)
{
	ID3D11DeviceContext* ctx = mD3dDeviceContext;

	// Create and grow vertex/index buffers if needed
	if (!g_pVB || g_VertexBufferSize < draw_data->TotalVtxCount)
	{
		if (g_pVB) { g_pVB->Release(); g_pVB = NULL; }
		g_VertexBufferSize = draw_data->TotalVtxCount + 5000;
		D3D11_BUFFER_DESC desc;
		memset(&desc, 0, sizeof(D3D11_BUFFER_DESC));
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = g_VertexBufferSize * sizeof(ImDrawVert);
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		if (mD3dDevice->CreateBuffer(&desc, NULL, &g_pVB) < 0)
			return;
	}
	if (!g_pIB || g_IndexBufferSize < draw_data->TotalIdxCount)
	{
		if (g_pIB) { g_pIB->Release(); g_pIB = NULL; }
		g_IndexBufferSize = draw_data->TotalIdxCount + 10000;
		D3D11_BUFFER_DESC desc;
		memset(&desc, 0, sizeof(D3D11_BUFFER_DESC));
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = g_IndexBufferSize * sizeof(ImDrawIdx);
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		if (mD3dDevice->CreateBuffer(&desc, NULL, &g_pIB) < 0)
			return;
	}

	// Copy and convert all vertices into a single contiguous buffer
	D3D11_MAPPED_SUBRESOURCE vtx_resource, idx_resource;
	if (ctx->Map(g_pVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &vtx_resource) != S_OK)
		return;
	if (ctx->Map(g_pIB, 0, D3D11_MAP_WRITE_DISCARD, 0, &idx_resource) != S_OK)
		return;
	ImDrawVert* vtx_dst = (ImDrawVert*)vtx_resource.pData;
	ImDrawIdx* idx_dst = (ImDrawIdx*)idx_resource.pData;
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		memcpy(vtx_dst, &cmd_list->VtxBuffer[0], cmd_list->VtxBuffer.size() * sizeof(ImDrawVert));
		memcpy(idx_dst, &cmd_list->IdxBuffer[0], cmd_list->IdxBuffer.size() * sizeof(ImDrawIdx));
		vtx_dst += cmd_list->VtxBuffer.size();
		idx_dst += cmd_list->IdxBuffer.size();
	}
	ctx->Unmap(g_pVB, 0);
	ctx->Unmap(g_pIB, 0);

	// Setup orthographic projection matrix into our constant buffer
	{
		D3D11_MAPPED_SUBRESOURCE mapped_resource;
		if (ctx->Map(g_pVertexConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource) != S_OK)
			return;
		VERTEX_CONSTANT_BUFFER* constant_buffer = (VERTEX_CONSTANT_BUFFER*)mapped_resource.pData;
		float L = 0.0f;
		float R = ImGui::GetIO().DisplaySize.x;
		float B = ImGui::GetIO().DisplaySize.y;
		float T = 0.0f;
		float mvp[4][4] =
		{
			{ 2.0f / (R - L),   0.0f,           0.0f,       0.0f },
			{ 0.0f,         2.0f / (T - B),     0.0f,       0.0f },
			{ 0.0f,         0.0f,           0.5f,       0.0f },
			{ (R + L) / (L - R),  (T + B) / (B - T),    0.5f,       1.0f },
		};
		memcpy(&constant_buffer->mvp, mvp, sizeof(mvp));
		ctx->Unmap(g_pVertexConstantBuffer, 0);
	}

	// Backup DX state that will be modified to restore it afterwards (unfortunately this is very ugly looking and verbose. Close your eyes!)
	struct BACKUP_DX11_STATE
	{
		UINT                        ScissorRectsCount, ViewportsCount;
		D3D11_RECT                  ScissorRects[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
		D3D11_VIEWPORT              Viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
		ID3D11RasterizerState*      RS;
		ID3D11BlendState*           BlendState;
		FLOAT                       BlendFactor[4];
		UINT                        SampleMask;
		ID3D11ShaderResourceView*   PSShaderResource;
		ID3D11SamplerState*         PSSampler;
		ID3D11PixelShader*          PS;
		ID3D11VertexShader*         VS;
		UINT                        PSInstancesCount, VSInstancesCount;
		ID3D11ClassInstance*        PSInstances[256], *VSInstances[256];   // 256 is max according to PSSetShader documentation
		D3D11_PRIMITIVE_TOPOLOGY    PrimitiveTopology;
		ID3D11Buffer*               IndexBuffer, *VertexBuffer, *VSConstantBuffer;
		UINT                        IndexBufferOffset, VertexBufferStride, VertexBufferOffset;
		DXGI_FORMAT                 IndexBufferFormat;
		ID3D11InputLayout*          InputLayout;
	};
	BACKUP_DX11_STATE old;
	old.ScissorRectsCount = old.ViewportsCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
	ctx->RSGetScissorRects(&old.ScissorRectsCount, old.ScissorRects);
	ctx->RSGetViewports(&old.ViewportsCount, old.Viewports);
	ctx->RSGetState(&old.RS);
	ctx->OMGetBlendState(&old.BlendState, old.BlendFactor, &old.SampleMask);
	ctx->PSGetShaderResources(0, 1, &old.PSShaderResource);
	ctx->PSGetSamplers(0, 1, &old.PSSampler);
	old.PSInstancesCount = old.VSInstancesCount = 256;
	ctx->PSGetShader(&old.PS, old.PSInstances, &old.PSInstancesCount);
	ctx->VSGetShader(&old.VS, old.VSInstances, &old.VSInstancesCount);
	ctx->VSGetConstantBuffers(0, 1, &old.VSConstantBuffer);
	ctx->IAGetPrimitiveTopology(&old.PrimitiveTopology);
	ctx->IAGetIndexBuffer(&old.IndexBuffer, &old.IndexBufferFormat, &old.IndexBufferOffset);
	ctx->IAGetVertexBuffers(0, 1, &old.VertexBuffer, &old.VertexBufferStride, &old.VertexBufferOffset);
	ctx->IAGetInputLayout(&old.InputLayout);

	// Setup viewport
	D3D11_VIEWPORT vp;
	memset(&vp, 0, sizeof(D3D11_VIEWPORT));
	vp.Width = ImGui::GetIO().DisplaySize.x;
	vp.Height = ImGui::GetIO().DisplaySize.y;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = vp.TopLeftY = 0.0f;
	ctx->RSSetViewports(1, &vp);

	// Bind shader and vertex buffers
	unsigned int stride = sizeof(ImDrawVert);
	unsigned int offset = 0;
	ctx->IASetInputLayout(g_pInputLayout);
	ctx->IASetVertexBuffers(0, 1, &g_pVB, &stride, &offset);
	ctx->IASetIndexBuffer(g_pIB, sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ctx->VSSetShader(g_pVertexShader, NULL, 0);
	ctx->VSSetConstantBuffers(0, 1, &g_pVertexConstantBuffer);
	ctx->PSSetShader(g_pPixelShader, NULL, 0);
	ctx->PSSetSamplers(0, 1, &g_pFontSampler);

	// Setup render state
	const float blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
	ctx->OMSetBlendState(g_pBlendState, blend_factor, 0xffffffff);
	ctx->RSSetState(g_pRasterizerState);

	// Render command lists
	int vtx_offset = 0;
	int idx_offset = 0;
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.size(); cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback)
			{
				pcmd->UserCallback(cmd_list, pcmd);
			}
			else
			{
				const D3D11_RECT r = { (LONG)pcmd->ClipRect.x, (LONG)pcmd->ClipRect.y, (LONG)pcmd->ClipRect.z, (LONG)pcmd->ClipRect.w };
				ctx->PSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)&pcmd->TextureId);
				ctx->RSSetScissorRects(1, &r);
				ctx->DrawIndexed(pcmd->ElemCount, idx_offset, vtx_offset);
			}
			idx_offset += pcmd->ElemCount;
		}
		vtx_offset += cmd_list->VtxBuffer.size();
	}

	// Restore modified DX state
	ctx->RSSetScissorRects(old.ScissorRectsCount, old.ScissorRects);
	ctx->RSSetViewports(old.ViewportsCount, old.Viewports);
	ctx->RSSetState(old.RS); if (old.RS) old.RS->Release();
	ctx->OMSetBlendState(old.BlendState, old.BlendFactor, old.SampleMask); if (old.BlendState) old.BlendState->Release();
	ctx->PSSetShaderResources(0, 1, &old.PSShaderResource); if (old.PSShaderResource) old.PSShaderResource->Release();
	ctx->PSSetSamplers(0, 1, &old.PSSampler); if (old.PSSampler) old.PSSampler->Release();
	ctx->PSSetShader(old.PS, old.PSInstances, old.PSInstancesCount); if (old.PS) old.PS->Release();
	for (UINT i = 0; i < old.PSInstancesCount; i++) if (old.PSInstances[i]) old.PSInstances[i]->Release();
	ctx->VSSetShader(old.VS, old.VSInstances, old.VSInstancesCount); if (old.VS) old.VS->Release();
	ctx->VSSetConstantBuffers(0, 1, &old.VSConstantBuffer); if (old.VSConstantBuffer) old.VSConstantBuffer->Release();
	for (UINT i = 0; i < old.VSInstancesCount; i++) if (old.VSInstances[i]) old.VSInstances[i]->Release();
	ctx->IASetPrimitiveTopology(old.PrimitiveTopology);
	ctx->IASetIndexBuffer(old.IndexBuffer, old.IndexBufferFormat, old.IndexBufferOffset); if (old.IndexBuffer) old.IndexBuffer->Release();
	ctx->IASetVertexBuffers(0, 1, &old.VertexBuffer, &old.VertexBufferStride, &old.VertexBufferOffset); if (old.VertexBuffer) old.VertexBuffer->Release();
	ctx->IASetInputLayout(old.InputLayout); if (old.InputLayout) old.InputLayout->Release();
}

ImguiHandler *imguiHandler;
