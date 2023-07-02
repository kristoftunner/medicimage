#if 0
#include "renderer/renderer.h"
#include "image_handling/image_loader.h"
//#include "backends/imgui_impl_sdl.h"
//#include "backends/imgui_impl_dx11.h"
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "core/log.h"
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include <iostream>

namespace medicimage
{

Renderer& Renderer::GetInstance()
{
  static Renderer s_instance;
  return s_instance;
}

void Renderer::Init(HWND window, const WindowProps& windowProperties)
{
  CreateDevice(window, windowProperties);  // TODO: do some error handling here(exception or return value and cleanup)
  CreateRenderTarget();

  m_viewPort = { 
      0.0f, 0.0f, 
      static_cast<float>(windowProperties.width), static_cast<float>(windowProperties.heigth), 
      0.0f, 1.0f 
  };
  InitShaders();
  SetupBuffers();
}

void Renderer::SwapBuffers()
{
  m_swapchain->Present(1,0);
}

void Renderer::Cleanup()
{
  CleanupDevice();
}

void Renderer::InitShaders()
{
  ThrowIfFailed(D3DCompileFromFile(L"assets/shaders/shaders.hlsl", 0, 0, "vs_main", "vs_5_0", 0, 0, &m_vsBlob, 0));
    
  ThrowIfFailed(m_device->CreateVertexShader(m_vsBlob->GetBufferPointer(),
                                      m_vsBlob->GetBufferSize(),
                                      0,
                                      &m_vs));
  ThrowIfFailed(D3DCompileFromFile(L"assets/shaders/shaders.hlsl", 0, 0, "ps_main", "ps_5_0", 0, 0, &m_psBlob, 0));
  ThrowIfFailed(m_device->CreatePixelShader(m_psBlob->GetBufferPointer(), m_psBlob->GetBufferSize(), 0, &m_ps));
}

void Renderer::SetupBuffers()
{
  D3D11_INPUT_ELEMENT_DESC InputElementDesc[] = {
      {
          "POSITION", 0, 
          DXGI_FORMAT_R32G32_FLOAT, 
          0, 0, 
          D3D11_INPUT_PER_VERTEX_DATA, 0
      },
      {
          "UV", 0, 
          DXGI_FORMAT_R32G32_FLOAT, 
          0, D3D11_APPEND_ALIGNED_ELEMENT, 
          D3D11_INPUT_PER_VERTEX_DATA, 0
      }
  };
    
  ThrowIfFailed(m_device->CreateInputLayout(InputElementDesc,ARRAYSIZE(InputElementDesc), m_vsBlob->GetBufferPointer(), m_vsBlob->GetBufferSize(), &m_inputLayout));
    
  float vertexData[] = {
      -1.0f,  -1.0f, 0.0f, 0.0f, 
      -1.0f,  1.0f,  0.0f, 1.0f,
      1.0f,   -1.0f, 1.0f, 0.0f,
      1.0f,   -1.0f, 1.0f, 0.0f,
      -1.0f,  1.0f,  0.0f, 1.0f,
      1.0f,   1.0f,  1.0f, 1.0f,
  };
    
  D3D11_BUFFER_DESC vertexDataDesc = {
      sizeof(vertexData),
      D3D11_USAGE_DEFAULT,
      D3D11_BIND_VERTEX_BUFFER,
      0, 0, 0
  };
  D3D11_SUBRESOURCE_DATA vertexDataInitial = { vertexData };
  ThrowIfFailed(m_device->CreateBuffer(&vertexDataDesc,
                                &vertexDataInitial,
                                &m_vertexBuffer));
  m_stride = 4 * sizeof(float);
  m_offset = 0;
  m_vertices = sizeof(vertexData) / m_stride;

}

static void GetHardwareAdapter(
  IDXGIFactory1* pFactory,
  IDXGIAdapter1** ppAdapter,
  bool requestHighPerformanceAdapter)
{
  *ppAdapter = nullptr;

  IDXGIAdapter1* adapter;

  IDXGIFactory6* factory6;
  ThrowIfFailed(pFactory->QueryInterface(IID_PPV_ARGS(&factory6)));
  
  if (!SUCCEEDED(factory6->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter))))
    APP_CORE_ERR("Havent find a high performance GPU for DirectX");
  
  DXGI_ADAPTER_DESC1 desc;
  adapter->GetDesc1(&desc);
  *ppAdapter = adapter;
}

void Renderer::CreateDevice(HWND hwnd, const WindowProps& windowProperties)
{
  // querying the devices
  UINT dxgiFactoryFlags = 0;

  IDXGIFactory4* factory;
  ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

  IDXGIAdapter1* hardwareAdapter;
  GetHardwareAdapter(factory, &hardwareAdapter, true);

  // Setup swap chain
  DXGI_SWAP_CHAIN_DESC sd;
  ZeroMemory(&sd, sizeof(sd));
  sd.BufferCount = 2;
  sd.BufferDesc.Width = windowProperties.width;
  sd.BufferDesc.Height = windowProperties.heigth;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = hwnd;
  sd.SampleDesc.Count = 1;
  sd.SampleDesc.Quality = 0;
  sd.Windowed = TRUE;
  sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

  UINT createDeviceFlags = 0;
  D3D_FEATURE_LEVEL featureLevel;
  const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
  ThrowIfFailed(D3D11CreateDevice(hardwareAdapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &m_device, &featureLevel, &m_deviceContext)); 
  ThrowIfFailed(factory->CreateSwapChain(m_device, &sd, &m_swapchain));
  //ThrowIfFailed(D3D11CreateDeviceAndSwapChain(hardwareAdapter, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_swapchain, &m_device, &featureLevel, &m_deviceContext)); 
  
  // for debug
  IDXGIDevice* deviceInterface = nullptr;
  ThrowIfFailed(m_device->QueryInterface(IID_PPV_ARGS(&deviceInterface)));
  IDXGIAdapter1* adapter = nullptr;
  ThrowIfFailed(deviceInterface->GetParent(IID_PPV_ARGS(&adapter)));
  DXGI_ADAPTER_DESC1 desc;
  adapter->GetDesc1(&desc);
  std::wstring gpuDescription(desc.Description);
  APP_CORE_INFO("Graphics card selected by DirectX:{}", std::string(gpuDescription.begin(), gpuDescription.end()));
}

void Renderer::Resize()
{
  CleanupRenderTarget();
  m_swapchain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
  CreateRenderTarget();
}

void Renderer::Draw()
{
  CleanupRenderTarget();
  CreateRenderTarget();
  float color[4] = {0.3f, 0.3f, 0.3f, 1.0f};
  m_deviceContext->ClearRenderTargetView(m_renderTargetView, color);
        
  m_deviceContext->RSSetViewports(1, &m_viewPort);
        
  m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, 0);

  m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  m_deviceContext->IASetInputLayout(m_inputLayout);
        
  m_deviceContext->VSSetShader(m_vs, 0, 0);
  m_deviceContext->PSSetShader(m_ps, 0, 0);
 
  m_deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &m_stride, &m_offset);

  m_deviceContext->Draw(m_vertices, 0);
}

void Renderer::CreateRenderTarget()
{
  ID3D11Texture2D* backBuffer = nullptr;
  ThrowIfFailed(m_swapchain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));
  ThrowIfFailed(m_device->CreateRenderTargetView(backBuffer, NULL, &m_renderTargetView));
  backBuffer->Release();
}

void Renderer::CleanupRenderTarget()
{
  if (m_renderTargetView) { m_renderTargetView->Release(); m_renderTargetView = nullptr; }
}

void Renderer::CleanupDevice()
{
  CleanupRenderTarget();
  if (m_swapchain) { m_swapchain->Release(); m_swapchain = nullptr; }
  if (m_deviceContext) { m_deviceContext->Release(); m_deviceContext = nullptr; }
  if (m_device) { m_device->Release(); m_device = nullptr; }
}
} // namespace medicimage

#endif