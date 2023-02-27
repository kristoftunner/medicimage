#include <iostream>

#include "event/event.h"
#include "key_event.h"
#include "event/application_event.h"
#include "window.h"
#include "backends/imgui_impl_sdl.h"
#include "backends/imgui_impl_dx11.h"
#include "core/log.h"
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

void EventInputHandler::FlushEvents()
{
  for(auto event : m_events)
    delete event;
  m_events.clear();
}

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
    else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(m_window))
    {
      APP_CORE_INFO("Close event recieved");
      m_events.push_back(new WindowCloseEvent());
    }
    else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED && event.window.windowID == SDL_GetWindowID(m_window))
    {
      APP_CORE_TRACE("Resize event recieved");
      Renderer::GetInstance().Resize(); // TODO: move this into an application layer, not the event generation stack
    }
    else if(event.type == SDL_TEXTINPUT)
    {
      // TODO: do UTF8 decoding here
      auto textInput = new KeyTextInputEvent(event.text.text); 
      m_events.push_back(textInput);
      APP_CORE_TRACE("Event received: {}", textInput->ToString());
    }
    else if(event.type == SDL_KEYDOWN)
    {
      auto keyDownEvent = new KeyPressedEvent(event.key.keysym.sym);  // TODO: proper mapping of SDL keys into KeyPressedEvent keymap
      m_events.push_back(keyDownEvent);
      APP_CORE_TRACE("Key pressed event received: {}", keyDownEvent->ToString());
    }
  }
}
  
} // namespace medicimage
