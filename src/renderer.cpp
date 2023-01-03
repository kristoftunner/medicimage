#include "renderer.h"
#include "backends/imgui_impl_sdl.h"
#include "backends/imgui_impl_dx11.h"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>

namespace medicimage
{
void Renderer::OnAttach()
{
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
  {
    printf("Error: %s\n", SDL_GetError());
    return;
  }

  // Setup window
  SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
  m_window = SDL_CreateWindow("Dear ImGui SDL2+DirectX11 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
  SDL_SysWMinfo wmInfo;
  SDL_VERSION(&wmInfo.version);
  SDL_GetWindowWMInfo(m_window, &wmInfo);
  HWND hwnd = (HWND)wmInfo.info.win.window;

  // Initialize Direct3D
  CreateDevice(hwnd); 
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  // Setup Platform/Renderer backends
  ImGui_ImplSDL2_InitForD3D(m_window);
  ImGui_ImplDX11_Init(m_device, m_deviceContext);
  
  LoadTexture();
}

void Renderer::OnDetach()
{
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  CleanupDevice();
  SDL_DestroyWindow(m_window);
  SDL_Quit();
}

void Renderer::OnRender()
{
  RenderBackground();
  
  // Start the Dear ImGui frame
  ImGui_ImplDX11_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();

  // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
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
    ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

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

  // Rendering
  ImGui::Render();

  //const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
  m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);
  //m_deviceContext->ClearRenderTargetView(m_renderTargetView, clear_color_with_alpha);
    
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

  m_swapchain->Present(1, 0); // Present with vsync
}

void Renderer::LoadTexture()
{
  cv::Mat image = cv::imread("Checkerboard.png");
  int matrixElementType = image.type();

  cv::resize(image, image, cv::Size(1280, 720));
  cv::cvtColor(image, image, cv::COLOR_BGR2RGBA);

  const int channelNumber = image.channels();
  const int numberOfPixels = image.total();
  const int pixelElementType = image.depth();
  const int pixelSizeInBytes = image.elemSize();
  const int pixelElementSizeInBytes = image.elemSize1();
  const int rows = image.rows;
  const int cols = image.cols;
  const int rowPitch = image.cols * pixelElementSizeInBytes;
  matrixElementType = image.type();
  //// Texture
  D3D11_TEXTURE2D_DESC ImageTextureDesc = {};
  
  ImageTextureDesc.Width = image.cols;
  ImageTextureDesc.Height = image.rows;
  ImageTextureDesc.MipLevels = 1;
  ImageTextureDesc.ArraySize = 1;
  ImageTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
  ImageTextureDesc.SampleDesc.Count = 1;
  ImageTextureDesc.SampleDesc.Quality = 0;
  ImageTextureDesc.Usage = D3D11_USAGE_IMMUTABLE;
  ImageTextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  
  D3D11_SUBRESOURCE_DATA ImageSubresourceData = {};
  ImageSubresourceData.pSysMem = image.ptr(); 
  ImageSubresourceData.SysMemPitch = rowPitch; 
    
  ThrowIfFailed(m_device->CreateTexture2D(&ImageTextureDesc, &ImageSubresourceData, &m_imageTexture));

}

void Renderer::RenderBackground()
{
  m_deviceContext->CopyResource(m_backBuffer, m_imageTexture);
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
