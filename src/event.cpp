#include <iostream>

#include "event.h"
#include "application_event.h"
#include "window.h"
#include "backends/imgui_impl_sdl.h"
#include "backends/imgui_impl_dx11.h"

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
      std::cout << "Quit event recieved" << std::endl;;
    }
    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(m_window))
    {
      std::cout << "Window close event revieved" << std::endl;
      m_events.push_back(new WindowCloseEvent());
    }
    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED && event.window.windowID == SDL_GetWindowID(m_window))
    {
      // Release all outstanding references to the swap chain's buffers before resizing.
      std::cout << "Window resize event recieved" << std::endl;
    }
  }
}
  
} // namespace medicimage
