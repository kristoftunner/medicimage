#include "imgui_layer.h"

#include "backends/imgui_impl_sdl.h"
#include "backends/imgui_impl_dx11.h"

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
  if (m_showDemoWindow)
    ImGui::ShowDemoWindow(&m_showDemoWindow);

  // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
  {
    static float f = 0.0f;
    static int counter = 0;

    ImGui::Begin("Hello, world!");              // Create a window called "Hello, world!" and append into it.

    ImGui::Text("This is some useful text.");         // Display some text (you can use a format strings too)
    ImGui::Checkbox("Demo Window", &m_showDemoWindow);    // Edit bools storing our window open/close state
    ImGui::Checkbox("Another Window", &m_showAnotherWindow);

    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);      // Edit 1 float using a slider from 0.0f to 1.0f
    ImGui::ColorEdit3("clear color", (float*)&m_clearColor); // Edit 3 floats representing a color

    if (ImGui::Button("Button"))              // Buttons return true when clicked (most widgets return true when edited/activated)
      counter++;
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();
  }

  // 3. Show another simple window.
  if (m_showAnotherWindow)
  {
    ImGui::Begin("Another Window", &m_showAnotherWindow);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
    ImGui::Text("Hello from another window!");
    if (ImGui::Button("Close Me"))
      m_showAnotherWindow = false;
    ImGui::End();
  }
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
}

} // namespace medicimage
