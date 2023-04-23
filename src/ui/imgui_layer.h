#pragma once

#include "core/layer.h"

#include "imgui.h"
#include <d3d11.h>
#include <SDL.h>
namespace medicimage
{

class ImguiLayer : public Layer
{
public:
  ImguiLayer(SDL_Window* window, ID3D11Device* d3dDevice, ID3D11DeviceContext* deviceContext);
  ~ImguiLayer();
  virtual void OnAttach() override;
  virtual void OnDetach() override;
  virtual void OnImguiRender() override;
  void Begin();
  void End();
private:
  bool m_showDemoWindow = true;
  bool m_showAnotherWindow = false;
  ImVec4 m_clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  
  // DirectX and SDL specific references needed for SDL IMGUI init
  ID3D11Device* m_device = nullptr;
  ID3D11DeviceContext* m_deviceContext = nullptr;
  SDL_Window* m_window;
};
  
} // namespace medicimage
