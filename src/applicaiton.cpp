#include "application.h"
#include "texture.h"
#include "log.h"

namespace medicimage
{

#define BIND_EVENT_FN(func) std::bind(&func, this, std::placeholders::_1)


Application::Application()
{
  Logger::Init(); // TODO: add this to some entrypoint
  m_window = std::unique_ptr<Window>(Window::Create(WindowProps()));

  Renderer& renderer = Renderer::GetInstance();
  m_imguiLayer = std::make_shared<ImguiLayer>(m_window->GetNativeWindow(), renderer.GetDevice(), renderer.GetDeviceContext());
  m_inputHandler = std::move(std::make_unique<EventInputHandler>(m_imguiLayer, m_window->GetNativeWindow()));
}

Application::~Application()
{
  for(auto event : m_inputHandler->GetCollectedEvents())
    delete event;
}

void Application::OnEvent(Event* e)
{
  EventDispatcher dispatcher(e);
  dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClosed));

  //for(auto it = m_layerStack.end(); it != m_layerStack.begin();)
  //{
  //  (*--it)->OnEvent(e);
  //  if(e.Handled())
  //    break;
  //}
}

bool Application::OnWindowClosed(WindowCloseEvent* e)
{
  m_running = false;
  return true;
}

void Application::Run()
{
  APP_CORE_INFO("Started application");
  auto& renderer = Renderer::GetInstance();
  m_imguiLayer->OnAttach();
  m_editor.OnAttach();
  while(m_running)
  {
    // Event handling
    auto texture = Texture2D("checkerboard", "Checkerboard.png");
    texture.Bind(0);
    renderer.Draw();

    // event handling for every frame
    m_inputHandler->PollEvents();
    auto events = m_inputHandler->GetCollectedEvents();
    for(auto event : events)
    {
      OnEvent(event); 
    }

    // Update the layers
    m_editor.OnUpdate();

    // ImGui rendering
    m_imguiLayer->Begin();
    m_editor.OnImguiRender();
    m_imguiLayer->End();

    renderer.SwapBuffers(); // this should be integrated into m_window->OnUpdate ??? idk..
    //m_window->OnUpdate(); 
  }
}
  
} // namespace medicimage
