#pragma once

#include <string>
#include <SDL.h>

namespace medicimage
{

struct WindowProps{
  std::string title;
  unsigned int width;
  unsigned int heigth;

  WindowProps(const std::string& title = "Gameng engine",
              unsigned int width = 1280, unsigned int heigth = 720)
            : title(title), width(width), heigth(heigth){}
};

class Window
{
public:
  Window(const WindowProps& windowProperties);
  ~Window();
  static Window* Create(const WindowProps& windowProperties);
  SDL_Window* GetNativeWindow(){ return m_window; }
private:
  void Init(const WindowProps& windowProperties);
  void ShutDown();
private:
  SDL_Window* m_window;
};

} // namespace medicimage
