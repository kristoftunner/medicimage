#include "medicimage.h"

using namespace medicimage;

int main(int, char**)
{
  // create a window, it initializes the renderer
  Window* window = Window::Create(WindowProps());
  Renderer& renderer = Renderer::GetInstance();
  ImguiLayer imguiLayer(window->GetNativeWindow(), renderer.GetDevice(), renderer.GetDeviceContext());
  imguiLayer.OnAttach();

  while(true)
  {
    imguiLayer.Begin();
    imguiLayer.OnImguiRender();
    imguiLayer.End();
    renderer.SwapBuffers();
  }


  return 0;
}