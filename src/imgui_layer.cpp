#include "imgui_layer.h"

#include "backends/imgui_impl_sdl.h"
#include "backends/imgui_impl_dx11.h"
#include "texture.h"
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
  
	// Note: Switch this to true to enable dockspace
	static bool dockspaceOpen = true;
	static bool opt_fullscreen_persistant = false;
	bool opt_fullscreen = opt_fullscreen_persistant;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    window_flags |= ImGuiWindowFlags_NoNavFocus;
	}

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
	ImGui::PopStyleVar();

	if (opt_fullscreen)
		ImGui::PopStyleVar(2);

	// DockSpace
	ImGuiIO& io = ImGui::GetIO();
	ImGuiStyle& style = ImGui::GetStyle();
	float minWinSizeX = style.WindowMinSize.x;
	style.WindowMinSize.x = 370.0f;
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}

  ImGui::End();

  ImGui::Begin("Currently captured frame window");   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
  auto texture = Texture2D("checkerboard","Checkerboard.png"); 
  ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
  ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
  ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
  ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
  ImVec2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available
  ImGui::Image(texture.GetShaderResourceView(), canvas_sz, uv_min, uv_max, tint_col, border_col);
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
