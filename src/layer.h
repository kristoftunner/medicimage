#pragma once 

#include <string>

namespace medicimage
{

class Layer {
public:
  Layer(const std::string& name = "Layer");
  virtual ~Layer();

  virtual void OnAttach() {}
  virtual void OnDetach() {}
  virtual void OnUpdate() {}
  virtual void OnEvent() {}
  virtual void OnImguiRender(){}

  inline const std::string& GetName() {return m_debugName; }
protected:
  std::string m_debugName;
};
  
} // namespace medicimage
