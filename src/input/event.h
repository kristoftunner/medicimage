#pragma once
#if 0
#include <memory>
#include <vector>
#include "ui/imgui_layer.h"
#include <functional>
#include <SDL.h>

#define BIT(x) (1<<x)

namespace medicimage
{

enum class EventType
{
  None = 0,
  WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
  AppTick, AppUpdate, AppRender,
  KeyPressed, KeyReleased, KeyTextInput, 
  MousePressed, MouseReleased,
  MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
};

enum EventCategory
{
  None = 0,
  EventCategoryApplication = BIT(0),
  EventCategoryInput = BIT(1),
  EventCategoryKeyboard = BIT(2),
  EventCategoryMouse = BIT(3),
  EventCategoryMouseButton = BIT(4)
};

#define EVENT_CLASS_TYPE(type) static EventType GetStaticType(){return EventType::type;} \
                              virtual EventType GetEventType() const override {return GetStaticType();} \
                              virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override {return category;}
#define BIND_EVENT_FN(func) std::bind(&func, this, std::placeholders::_1)

class Event {
  friend class EventDispatcher;
public:
  Event() = default;
  ~Event(){}
  virtual EventType GetEventType() const = 0;
  virtual const char* GetName() const = 0;
  virtual int GetCategoryFlags() const = 0;
  virtual std::string ToString() const {return GetName();};
  inline bool IsInCategory(EventCategory category)
  {
    return GetCategoryFlags() & category;
  }
  bool Handled(){return m_handled; }
protected:
  bool m_handled = false;
};

class EventDispatcher
{

template<typename T>
using EventFn = std::function<bool(T)>;
public:
  EventDispatcher(Event* event) : m_event(event){}
  template<typename T>
  bool Dispatch(EventFn<T*> func)
  {
    if(m_event->GetEventType() == T::GetStaticType())
    {
      m_event->m_handled = func((T*)m_event);
      return true;
    }
    else 
      return false;
  }

private:
  Event* m_event; // TODO: shared_ptr or something here needed in the future
};


class EventInputHandler 
{
public:
  EventInputHandler(SDL_Window* window);
  ~EventInputHandler();
  void PollEvents();
  void FlushEvents();
  std::vector<Event*>& GetCollectedEvents(){return m_events;}
private:
  SDL_Window* m_window;
  std::vector<Event*> m_events;
};

} // namespace medicimage
#endif