#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <memory>
#include <Windows.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <stdexcept>
#include "imgui.h"

namespace medicimage
{

class Renderer
{
public:
  Renderer() = default;
  ~Renderer(){} // TODO: do some unique_ptr or ComPtr magic on the destruction
  void OnAttach();
  void OnDetach();
  void OnRender();
private:
  void CreateDevice(HWND hwnd);
  void CreateRenderTarget();
  void CleanupDevice();
  void CleanupRenderTarget();
   
// TODO: these should be as well exported somewhere
  void RenderBackground();
  void LoadTexture(); 
private:
  ID3D11Device* m_device = nullptr;
  ID3D11DeviceContext* m_deviceContext = nullptr;
  IDXGISwapChain*      m_swapchain = nullptr;
  ID3D11RenderTargetView*  m_renderTargetView = nullptr;
  ID3D11Texture2D* m_backBuffer = nullptr;

  ID3D11Texture2D* m_imageTexture = nullptr;

  SDL_Window* m_window;
  // gui specific variables, TODO: move these later into some GUI class
  bool m_showDemoWindow = false;
  bool m_showAnotherWindow = false;
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
};

inline std::string HrToString(HRESULT hr)
{
    char s_str[64] = {};
    sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
    return std::string(s_str);
}

class HrException : public std::runtime_error
{
public:
    HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
    HRESULT Error() const { return m_hr; }
private:
    const HRESULT m_hr;
};

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw HrException(hr);
    }
}

} // namespace medicimage
