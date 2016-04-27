
#include <d3dcompiler.h>
#include <windows.h>
#include <d3d11.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include "imguiHandler.h"
#include "RayTracer.h"
#include "Octree.h"

#define SAFE_RELEASE(x) if((x)){(x)->Release();(x) = nullptr;}

class Octree_renderer
{
	ID3D11Buffer *vertexBuffer;
	ID3D11Buffer *constantBuffer;
	ID3D11VertexShader *vertexShader;
	ID3D11PixelShader *pixelShader;
	ID3D11InputLayout *inputLayout;
	D3D11_VIEWPORT viewPort;

	void RenderNode(ID3D11DeviceContext *d3dDeviceContext, OctreeNode *node)
	{
		glm::vec3 positions[32];

		float minX = node->minCoordinates.x;
		float minY = node->minCoordinates.y;
		float minZ = node->minCoordinates.z;
		float maxX = node->maxCoordinates.x;
		float maxY = node->maxCoordinates.y;
		float maxZ = node->maxCoordinates.z;
		
		int i = 0;
		positions[i++] = glm::vec3(minX, minY, minZ);
		positions[i++] = glm::vec3(minX, maxY, minZ);

		positions[i++] = glm::vec3(minX, maxY, minZ);
		positions[i++] = glm::vec3(minX, maxY, maxZ);

		positions[i++] = glm::vec3(minX, maxY, maxZ);
		positions[i++] = glm::vec3(minX, minY, maxZ);

		positions[i++] = glm::vec3(minX, minY, maxZ);
		positions[i++] = glm::vec3(minX, minY, minZ);



		positions[i++] = glm::vec3(minX, minY, minZ);
		positions[i++] = glm::vec3(maxX, minY, minZ);

		positions[i++] = glm::vec3(maxX, minY, minZ);
		positions[i++] = glm::vec3(maxX, minY, maxZ);

		positions[i++] = glm::vec3(maxX, minY, maxZ);
		positions[i++] = glm::vec3(minX, minY, maxZ);

		positions[i++] = glm::vec3(minX, minY, maxZ);
		positions[i++] = glm::vec3(minX, minY, minZ);



		positions[i++] = glm::vec3(maxX, minY, minZ);
		positions[i++] = glm::vec3(maxX, maxY, minZ);

		positions[i++] = glm::vec3(maxX, maxY, minZ);
		positions[i++] = glm::vec3(maxX, maxY, maxZ);

		positions[i++] = glm::vec3(maxX, maxY, maxZ);
		positions[i++] = glm::vec3(maxX, minY, maxZ);

		positions[i++] = glm::vec3(maxX, minY, maxZ);
		positions[i++] = glm::vec3(maxX, minY, minZ);



		positions[i++] = glm::vec3(maxX, maxY, minZ);
		positions[i++] = glm::vec3(maxX, maxY, maxZ);

		positions[i++] = glm::vec3(maxX, maxY, maxZ);
		positions[i++] = glm::vec3(minX, maxY, maxZ);

		positions[i++] = glm::vec3(minX, maxY, maxZ);
		positions[i++] = glm::vec3(minX, maxY, minZ);

		positions[i++] = glm::vec3(minX, maxY, minZ);
		positions[i++] = glm::vec3(maxX, maxY, minZ);

		D3D11_MAPPED_SUBRESOURCE msr;
		d3dDeviceContext->Map(vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
		memcpy(msr.pData, positions, sizeof(glm::vec3) * 32);
		d3dDeviceContext->Unmap(vertexBuffer, 0);

		d3dDeviceContext->Draw(32, 0);

		if (!node->isLeafNode)
		{
			for (int i_child = 0; i_child < 8; i_child++)
			{
				RenderNode(d3dDeviceContext, node->childNodes[i_child]);
			}
		}		
	}

public:
	Octree_renderer(ID3D11Device *d3dDevice, int screenWidth, int screenHeight)
	{
		vertexBuffer = nullptr;
		constantBuffer = nullptr;
		vertexShader = nullptr;
		pixelShader = nullptr;

		viewPort.Height = screenHeight;
		viewPort.MaxDepth = 1.0f;
		viewPort.MinDepth = 0;
		viewPort.TopLeftX = 0;
		viewPort.TopLeftY = 0;
		viewPort.Width = screenWidth;

		{
			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.ByteWidth = sizeof(glm::vec3) * 32;
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			d3dDevice->CreateBuffer(&bd, nullptr, &vertexBuffer);
		}
		{
			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
			bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bd.ByteWidth = sizeof(glm::mat4x4);
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			d3dDevice->CreateBuffer(&bd, nullptr, &constantBuffer);
		}

		{
			const char *ps =
				"float4 main_ps() : SV_Target"
				"{"
				"	return float4(1,0,1,1);"
				"}";
			ID3DBlob *compiledCode;
			ID3DBlob *errorCode;
			D3DCompile(ps, strlen(ps), nullptr, nullptr, nullptr, "main_ps", "ps_4_0", 0, 0, &compiledCode, &errorCode);
			d3dDevice->CreatePixelShader(compiledCode->GetBufferPointer(), compiledCode->GetBufferSize(), nullptr, &pixelShader);
		}
		{
			const char *vs =
				"cbuffer asd: register(b0){float4x4 viewProj;};"
				"float4 main_vs(float3 position : POSITION) : SV_Position"
				"{"
				"	return mul(viewProj, float4(position,1.0));"
				"}";
			ID3DBlob *compiledCode;
			ID3DBlob *errorCode;
			D3DCompile(vs, strlen(vs), nullptr, nullptr, nullptr, "main_vs", "vs_4_0", 0, 0, &compiledCode, &errorCode);
			d3dDevice->CreateVertexShader(compiledCode->GetBufferPointer(), compiledCode->GetBufferSize(), nullptr, &vertexShader);
		
			D3D11_INPUT_ELEMENT_DESC desc = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
			d3dDevice->CreateInputLayout(&desc, 1, compiledCode->GetBufferPointer(), compiledCode->GetBufferSize(), &inputLayout);
		}
	}

	void Render(ID3D11DeviceContext *d3dDeviceContext, Octree *octree, const glm::mat4x4 &viewProj)
	{
		d3dDeviceContext->IASetInputLayout(inputLayout);
		UINT stride = sizeof(glm::vec3);
		UINT offset = 0;
		d3dDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		d3dDeviceContext->VSSetShader(vertexShader, 0, 0);
		d3dDeviceContext->PSSetShader(pixelShader, 0, 0);
		d3dDeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

		D3D11_MAPPED_SUBRESOURCE msr;
		d3dDeviceContext->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
		memcpy(msr.pData, &viewProj, sizeof(glm::mat4x4));
		d3dDeviceContext->Unmap(constantBuffer, 0);

		d3dDeviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);
		d3dDeviceContext->RSSetViewports(1, &viewPort);

		RenderNode(d3dDeviceContext, octree->GetRootNode());
	}
};

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

float RandomFloat()
{
	return rand() / (float)RAND_MAX;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	//////////////////////////////////////////////////////////////////////////
	//Create window
	//////////////////////////////////////////////////////////////////////////
	int screenWidth = 1280;
	int screenHeight = 720;

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
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.Height = screenHeight;
	scd.BufferDesc.Width = screenWidth;
	scd.BufferDesc.RefreshRate.Numerator = 60;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT|DXGI_USAGE_UNORDERED_ACCESS;
	scd.OutputWindow = windowHandle;
	scd.SampleDesc.Count = 1;
	scd.Windowed = TRUE;
	D3D_FEATURE_LEVEL outLevel;
	D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, deviceFlag, &levels, 1, D3D11_SDK_VERSION,
		&scd, &d3dSwapChain, &d3dDevice, &outLevel, &d3dDeviceContext);
	ID3D11Texture2D *screenTexture;
	ID3D11RenderTargetView *screenTextureRTV;
	ID3D11UnorderedAccessView *screenTextureUAV;
	d3dSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&screenTexture);
	d3dDevice->CreateRenderTargetView(screenTexture, nullptr, &screenTextureRTV);
	d3dDevice->CreateUnorderedAccessView(screenTexture, nullptr, &screenTextureUAV);
	d3dDeviceContext->OMSetRenderTargets(1, &screenTextureRTV, nullptr);
	//////////////////////////////////////////////////////////////////////////
	//UI
	//////////////////////////////////////////////////////////////////////////

	imguiHandler = new ImguiHandler(d3dDeviceContext, d3dDevice, windowHandle);
	imguiHandler->Init();

	RayTracer tracer;

	glm::vec3 camPos(-25,-25,10);
	glm::vec3 targetPos(0);
	glm::vec3 camUp(0, 0, 1);
	float verticalFov = 25;
	tracer.SetCameraParams(camPos, targetPos, camUp, screenWidth, screenHeight, verticalFov);

	unsigned int pixelCount = screenWidth * screenHeight;
	const float *srcPtr = tracer.GetRenderTargetBuffer();
	unsigned char *dstPointer = new unsigned char[pixelCount * 4];

	ID3D11ComputeShader *computeShader;
	{
		ID3DBlob *compiledBlod;
		ID3DBlob *errorMessage;
		D3DCompileFromFile(L"ComputeShader.compute", nullptr, nullptr, "main", "cs_5_0", 0, 0, &compiledBlod, &errorMessage);
		if (errorMessage)
		{
				const char *asd = (const char *)errorMessage->GetBufferPointer();
				__debugbreak();
		}
		d3dDevice->CreateComputeShader(compiledBlod->GetBufferPointer(), compiledBlod->GetBufferSize(), nullptr, &computeShader);
	}

	ID3D11ShaderResourceView *lightBufferSRV = nullptr;
	ID3D11ShaderResourceView *sphereBufferSRV = nullptr;
	ID3D11ShaderResourceView *octreeBufferSRV = nullptr;
	ID3D11Buffer *constantBufffer = nullptr;

	Octree_renderer renderer(d3dDevice, screenWidth, screenHeight);
	glm::mat4x4 viewMatrix = glm::lookAtRH(camPos, targetPos, camUp);
	glm::mat4x4 projMatrix = glm::perspectiveRH(glm::radians(verticalFov*2), (float)screenWidth /(float)screenHeight, 0.01f, 1000.0f);
	glm::mat4x4 viewProj = projMatrix * viewMatrix;
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	while (msg.message != WM_QUIT)
	{
		if (imguiHandler->rebuildRequested)
		{
			tracer.Clear();
			SAFE_RELEASE(lightBufferSRV);
			SAFE_RELEASE(sphereBufferSRV);
			SAFE_RELEASE(octreeBufferSRV);
			SAFE_RELEASE(constantBufffer);
			float minRadius = imguiHandler->minSphereRadiuses;
			float maxRadius = imguiHandler->maxSphereRadiuses;
			glm::vec3 maxSpherePos(20, 20, 5);
			int sphereCount = imguiHandler->sphereCount;
			for (int i_sphere = 0; i_sphere < sphereCount; i_sphere++)
			{
				glm::vec3 sphereCenter;
				float sphereRad;
				if (!imguiHandler->sphereOverlap)
				{
					bool validSphere = true;
					while (1)
					{
						sphereCenter = glm::vec3(RandomFloat(), RandomFloat(), RandomFloat());
						sphereCenter *= maxSpherePos;
						sphereRad = RandomFloat() * (maxRadius - minRadius) + minRadius;

						for (int i_sphere = 0; i_sphere < tracer.mSpheres.size(); i_sphere++)
						{
							float distance = glm::length(tracer.mSpheres[i_sphere]->center - sphereCenter);
							if (distance < tracer.mSpheres[i_sphere]->radius + sphereRad)
							{
								validSphere = false;
								break;
							}
						}

						if (validSphere)
						{
							break;
						}
					}
				}
				else
				{
					sphereCenter = glm::vec3(RandomFloat(), RandomFloat(), RandomFloat());
					sphereCenter *= maxSpherePos;
					sphereRad = RandomFloat() * (maxRadius - minRadius) + minRadius;
				}
				glm::vec4 sphereColor(RandomFloat(), RandomFloat(), RandomFloat(), 1);
				Sphere *newSphere = new Sphere(sphereRad, sphereCenter, sphereColor);
				tracer.AddSphere(newSphere);
			}

			Light light1;
			light1.color = glm::vec4(1000, 1000, 1000, 1);
			light1.position = glm::vec3(0, 0, 15);

			tracer.AddLight(light1);
			tracer.Update();
			tracer.CreateGpuBuffers(d3dDevice, &octreeBufferSRV, &lightBufferSRV, &sphereBufferSRV, &constantBufffer);
			
			ID3D11RenderTargetView *nullRTV = nullptr;
			d3dDeviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);

			d3dDeviceContext->CSSetShader(computeShader, nullptr, 0);
			d3dDeviceContext->CSSetUnorderedAccessViews(0, 1, &screenTextureUAV, nullptr);
			d3dDeviceContext->CSSetShaderResources(0, 1, &sphereBufferSRV);
			d3dDeviceContext->CSSetShaderResources(1, 1, &lightBufferSRV);
			d3dDeviceContext->CSSetShaderResources(2, 1, &octreeBufferSRV);
			d3dDeviceContext->CSSetConstantBuffers(0, 1, &constantBufffer);
			d3dDeviceContext->Dispatch(screenWidth / 16, screenHeight / 16, 1);

			//renderer.Render(d3dDeviceContext, &tracer.octree, viewProj);

			for (int i_pixel = 0; i_pixel < pixelCount; i_pixel++)
			{
				unsigned int pixelStart = i_pixel * 4;
				dstPointer[pixelStart + 0] = glm::clamp(srcPtr[pixelStart + 0], 0.0f, 1.f) * 255.0f;
				dstPointer[pixelStart + 1] = glm::clamp(srcPtr[pixelStart + 1], 0.0f, 1.f) * 255.0f;
				dstPointer[pixelStart + 2] = glm::clamp(srcPtr[pixelStart + 2], 0.0f, 1.f) * 255.0f;
				dstPointer[pixelStart + 3] = glm::clamp(srcPtr[pixelStart + 3], 0.0f, 1.f) * 255.0f;
			}

			imguiHandler->rebuildRequested = false;
		}

		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}
		//d3dDeviceContext->UpdateSubresource(screenTexture, 0, nullptr, dstPointer, sizeof(unsigned char) * 4 * screenWidth, 0);
 		imguiHandler->StartNewFrame();
 		imguiHandler->Render();

		d3dSwapChain->Present(0, 0);
	}

	return 0;
}