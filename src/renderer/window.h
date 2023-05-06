#pragma once
#if 0

#include <string>
#include <SDL.h>

namespace medicimage
{

struct WindowProps{
  std::string title;
  unsigned int width;
  unsigned int heigth;

  WindowProps(const std::string& title = "Medical imaging",
              unsigned int width = 1280, unsigned int heigth = 645)
            : title(title), width(width), heigth(heigth){}
};

class Window
{
public:
  Window(const WindowProps& windowProperties);
  ~Window();
  static Window* Create(const WindowProps& windowProperties);
  SDL_Window* GetNativeWindow(){ return m_window; }
  int GetWindowHeight()
  {
    int widht,height;
    SDL_GetWindowSize(m_window, &widht, &height);
    return height;
  }
  int GetWindowWidth()
  {
    int width,height;
    SDL_GetWindowSize(m_window, &width, &height);
    return width;
  }
private:
  void Init(const WindowProps& windowProperties);
  void ShutDown();
private:
  SDL_Window* m_window;
  WindowProps m_windowProperties;
};

} // namespace medicimage
#endif