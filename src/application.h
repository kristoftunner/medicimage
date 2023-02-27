#pragma once

#include "event/application_event.h"
#include "imgui_layer.h"
#include "window.h"
#include "renderer.h"
#include "event/event.h"
#include "editor_ui.h"

namespace medicimage
{
  class Application{
  public:
    Application();
    ~Application();
    void Run();
    void OnEvent(Event* e);
    inline Window& GetWindow(){ return *m_window; }
  private:
    bool OnWindowClosed(WindowCloseEvent* e);
    std::unique_ptr<EventInputHandler> m_inputHandler;
    std::unique_ptr<Window> m_window;
    std::shared_ptr<ImguiLayer> m_imguiLayer;
    EditorUI m_editor;
    bool m_running = true;
    float m_lastFrameTime = 0;
  };
  
} // namespace medicimage
