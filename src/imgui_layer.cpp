#include "imgui_layer.h"

#include "backends/imgui_impl_sdl.h"
#include "backends/imgui_impl_dx11.h"
#include "texture.h"
#include <iostream>

namespace medicimage
{

ImguiLayer::ImguiLayer(SDL_Window* window, ID3D11Device* d3dDevice, ID3D11DeviceContext* deviceContext) 
  : Layer("ImguiLayer"), m_window(window), m_device(d3dDevice), m_deviceContext(deviceContext)
{
  if(window == nullptr || d3dDevice != nullptr || deviceContext != nullptr)
  {
    //TODO: do some error handling
  }
}

ImguiLayer::~ImguiLayer()
{

} 

void ImguiLayer::OnAttach()
{
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // Setup Platform/Renderer backends
  bool ret = ImGui_ImplSDL2_InitForD3D(m_window);
  ret = ImGui_ImplDX11_Init(m_device, m_deviceContext);
} 

void ImguiLayer::OnDetach()
{
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
} 

void ImguiLayer::OnImguiRender()
{
  // DockSpace
  static bool dockspaceOpen = true;
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("DockSpace Demo", &dockspaceOpen, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking);
  ImGui::PopStyleVar();
  ImGuiIO& io = ImGui::GetIO();
  ImGuiStyle& style = ImGui::GetStyle();
  //float minWinSizeX = style.WindowMinSize.x;
  //style.WindowMinSize.x = 370.0f;
  if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
  {
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
  }
  ImGui::End();

  // Currently captured frame editing
  ImGui::Begin("Currently captured frame window");   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
  auto texture = Texture2D("checkerboard","Checkerboard.png"); 
  ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
  ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
  ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
  ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
  ImVec2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available
  ImGui::Image(texture.GetShaderResourceView(), canvas_sz, uv_min, uv_max, tint_col, border_col);
  ImGui::End();

  // Picture thumbnails
  bool openThumbnails = true;
  ImGui::Begin("Thumbnails", &openThumbnails, ImGuiWindowFlags_HorizontalScrollbar);
  for (int i = 0; i < 30; i++)
  {
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    float aspectRatio = texture.GetWidth() / texture.GetHeight();
    ImGui::Image(texture.GetShaderResourceView(), ImVec2{canvasSize.x, canvasSize.x / aspectRatio}, uv_min, uv_max, tint_col, border_col);
  }
  ImGui::End();
  
  bool openTools = true;
  ImGui::Begin("Tools", &openTools , ImGuiWindowFlags_HorizontalScrollbar);
  for (int i = 0; i < 8; i++)
  {
    // UV coordinates are often (0.0f, 0.0f) and (1.0f, 1.0f) to display an entire textures.
    // Here are trying to display only a 32x32 pixels area of the texture, hence the UV computation.
    // Read about UV coordinates here: https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples
    ImGui::PushID(i);
    if (i > 0)
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(i - 1.0f, i - 1.0f));
    ImVec2 size = ImVec2(32.0f, 32.0f);                         // Size of the image we want to make visible
    ImVec4 bg_col = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);             // Black background
    ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);           // No tint
    style.WindowMinSize.x = size.x *1.5;
    if (ImGui::ImageButton("", texture.GetShaderResourceView(), size, uv_min, uv_max, bg_col, tint_col))
      std::cout << "Pressed my button" << std::endl;
    if (i > 0)
        ImGui::PopStyleVar();
    ImGui::PopID();
  }
  
  ImGui::End();
} 

void ImguiLayer::Begin()
{
  ImGui_ImplDX11_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();
}

void ImguiLayer::End()
{
  ImGui::Render();
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	ImGuiIO& io = ImGui::GetIO();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
  {
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
  }
}

} // namespace medicimage
