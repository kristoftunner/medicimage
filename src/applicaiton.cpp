#if 0
#include "application.h"
#include "core/log.h"
#include "renderer/texture.h"

#include <ctime>
namespace medicimage
{
Application::Application()
{
  Logger::Init(); // TODO: add this to some entrypoint
  m_window = std::unique_ptr<Window>(Window::Create(WindowProps()));

  Renderer& renderer = Renderer::GetInstance();
  m_imguiLayer = std::make_shared<ImguiLayer>(m_window->GetNativeWindow(), renderer.GetDevice(), renderer.GetDeviceContext());
  m_inputHandler = std::make_unique<EventInputHandler>(m_window->GetNativeWindow());
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

static bool ShouldExit()
{
  std::time_t result = std::time(nullptr);
  auto time = std::gmtime(&result);

  if ((time->tm_year == 123) && (time->tm_mon < (8)))
    if((time->tm_mday < 31) || (time->tm_mon < 1))
      return false;
  else
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
    // Event handling;
    auto image = Image2D("assets/textures/Checkerboard.png");
    renderer.Draw();
    // simple check for exiting the app
    if (ShouldExit())
    {
      APP_CORE_ERR("Your time is up! please buy me");
      break;
    }

    // event handling for every frame
    m_inputHandler->PollEvents();
    auto events = m_inputHandler->GetCollectedEvents();
    for(auto event : events)
    {
      OnEvent(event);
      m_editor.OnEvent(event);
    }
    m_inputHandler->FlushEvents();

    // Update the layers
    m_editor.OnUpdate();

    // ImGui rendering
    m_imguiLayer->Begin();
    m_editor.OnImguiRender();
    m_imguiLayer->End();

    renderer.SwapBuffers(); // TODO: this should be integrated into m_window->OnUpdate ??? idk..
    //m_window->OnUpdate(); 
  }
}
  
} // namespace medicimage
#endif