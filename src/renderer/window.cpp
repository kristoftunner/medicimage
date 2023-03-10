#include "renderer/renderer.h"
#include "renderer/window.h"

#include <iostream>
#include <SDL_syswm.h>

namespace medicimage
{

Window* Window::Create(const WindowProps& windowProperties = WindowProps())
{
  Window* window = new Window(windowProperties);
  return window;  
}

Window::Window(const WindowProps& windowProperties)
{
  Init(windowProperties);
}

Window::~Window()
{
  ShutDown();
}

void Window::Init(const WindowProps& windowProperties)
{
  SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
  m_window = SDL_CreateWindow(windowProperties.title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowProperties.width, windowProperties.heigth, window_flags);
  SDL_SysWMinfo wmInfo;
  SDL_VERSION(&wmInfo.version);
  SDL_GetWindowWMInfo(m_window, &wmInfo);
  HWND hwnd = (HWND)wmInfo.info.win.window;

  Renderer::GetInstance().Init(hwnd, windowProperties); 
}

void Window::ShutDown()
{
  Renderer::GetInstance().Cleanup();
  SDL_DestroyWindow(m_window);
  SDL_Quit();
}


} // namespace medicimage
