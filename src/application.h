#pragma once

#include "input/application_event.h"
#include "ui/imgui_layer.h"
#include "ui/editor_ui.h"
#include "renderer/renderer.h"
#include "input/event.h"

#include "renderer/window.h"

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
