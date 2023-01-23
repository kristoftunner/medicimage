#include <iostream>

#include "event.h"
#include "application_event.h"
#include "window.h"
#include "backends/imgui_impl_sdl.h"
#include "backends/imgui_impl_dx11.h"
#include "log.h"
#include "renderer.h"

namespace medicimage
{

EventInputHandler::EventInputHandler(std::shared_ptr<ImguiLayer> imgui, SDL_Window* window)
  : m_imguiLayer(imgui), m_window(window)
{
  if(window == nullptr)
    std::cout << "EventHandler got nullptr window" << std::endl;
}

EventInputHandler::~EventInputHandler()
{}

void EventInputHandler::PollEvents()
{
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    ImGui_ImplSDL2_ProcessEvent(&event);
    if (event.type == SDL_QUIT)
    {
      APP_CORE_INFO("Quit event recieved");
    }
    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(m_window))
    {
      APP_CORE_INFO("Close event recieved");
      m_events.push_back(new WindowCloseEvent());
    }
    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED && event.window.windowID == SDL_GetWindowID(m_window))
    {
      // Release all outstanding references to the swap chain's buffers before resizing.
      APP_CORE_INFO("Resize event recieved");
      Renderer::GetInstance().Resize();
    }
  }
}
  
} // namespace medicimage
