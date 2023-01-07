#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <Windows.h>
#include "layer.h"
#include <stdexcept>

namespace medicimage
{

class Renderer 
// TODO: create a renderer API class and move swapchain->Present() and init to there
{
public:
  Renderer() = default;  
  void Init(HWND window);
  void SwapBuffers();
  void Cleanup();

  ID3D11Device* GetDevice(){return m_device;}
  ID3D11DeviceContext* GetDeviceContext() {return m_deviceContext; }
  IDXGISwapChain* GetSwapChain(){return m_swapchain;}
  static Renderer& GetInstance(); // TODO: think about this singleton design concept once more
private:
  void CreateDevice(HWND hwnd);
  void CreateRenderTarget();
  void CleanupDevice();
  void CleanupRenderTarget();
   
private:
  static Renderer s_instance;
  ID3D11Device* m_device = nullptr;
  ID3D11DeviceContext* m_deviceContext = nullptr;
  IDXGISwapChain*      m_swapchain = nullptr;
  ID3D11RenderTargetView*  m_renderTargetView = nullptr;
  ID3D11Texture2D* m_backBuffer = nullptr;
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

