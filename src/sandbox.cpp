#include "renderer.h"

int main(int, char**)
{
  medicimage::Renderer renderer;
  renderer.OnAttach();

  while(true)
  {
    renderer.OnRender();
  }
  return 0;
}