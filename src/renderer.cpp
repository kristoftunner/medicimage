#include "renderer.h"
#include "backends/imgui_impl_sdl.h"
#include "backends/imgui_impl_dx11.h"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>

namespace medicimage
{



Renderer& Renderer::GetInstance()
{
  static Renderer s_instance;
  return s_instance;
}

void Renderer::Init(HWND window)
{
  CreateDevice(window);  // TODO: do some error handling here(exception or return value and cleanup)
}

void Renderer::SwapBuffers()
{
  m_swapchain->Present(1,0);
  m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);
}

void Renderer::Cleanup()
{
  CleanupDevice();
}

void Renderer::CreateDevice(HWND hwnd)
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
  sd.OutputWindow = hwnd;
  sd.SampleDesc.Count = 1;
  sd.SampleDesc.Quality = 0;
  sd.Windowed = TRUE;
  sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

  UINT createDeviceFlags = 0;
  //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
  D3D_FEATURE_LEVEL featureLevel;
  const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
  ThrowIfFailed(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_swapchain, &m_device, &featureLevel, &m_deviceContext)); 

  CreateRenderTarget();
}

void Renderer::CreateRenderTarget()
{
  ThrowIfFailed(m_swapchain->GetBuffer(0, IID_PPV_ARGS(&m_backBuffer)));
  ThrowIfFailed(m_device->CreateRenderTargetView(m_backBuffer, NULL, &m_renderTargetView));
}

void Renderer::CleanupRenderTarget()
{
  if (m_renderTargetView) { m_renderTargetView->Release(); m_renderTargetView = nullptr; }
  if (m_backBuffer) { m_backBuffer->Release(); m_backBuffer = nullptr; }
}

void Renderer::CleanupDevice()
{
  CleanupRenderTarget();
  if (m_swapchain) { m_swapchain->Release(); m_swapchain = nullptr; }
  if (m_deviceContext) { m_deviceContext->Release(); m_deviceContext = nullptr; }
  if (m_device) { m_device->Release(); m_device = nullptr; }
}
} // namespace medicimage

