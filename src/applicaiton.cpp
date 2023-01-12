#include "application.h"
#include "texture.h"
#include "log.h"

namespace medicimage
{

#define BIND_EVENT_FN(func) std::bind(&func, this, std::placeholders::_1)


Application::Application()
{
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
  Logger::Init(); // TODO: add this to some entrypoint
  APP_CORE_INFO("Started application");
  auto& renderer = Renderer::GetInstance();
  m_imguiLayer->OnAttach();
  while(m_running)
  {
    // Event handling
    m_inputHandler->PollEvents();
    auto events = m_inputHandler->GetCollectedEvents();
    auto texture = Texture2D("checkerboard", "Checkerboard.png");
    texture.Bind(0);
    renderer.Draw();

    for(auto event : events)
    {
      OnEvent(event); 
    }
    //float time = static_cast<float>(glfwGetTime()); // TODO REFACTOR: Platform::GetTime()
    //Timestep timestep = time - m_lastFrameTime; 
    //m_lastFrameTime = time;   
    //for(Layer* layer : m_layerStack)
    // 
    // 
    //{
    //  layer->OnUpdate(timestep);
    //}

    m_imguiLayer->Begin();

    m_imguiLayer->OnImguiRender();
    //for(Layer* layer : m_layerStack)
    //{
    //  layer->OnImguiRender();
    //}
    m_imguiLayer->End();
    renderer.SwapBuffers(); // this should be integrated into m_window ???
    //m_window->OnUpdate(); 
  }
}
  
} // namespace medicimage
