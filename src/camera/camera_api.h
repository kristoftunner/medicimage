#pragma once
#include "renderer/texture.h"

#include <memory>
#include <optional>
namespace medicimage
{

// Simple camera interface, one thing is sure: we need the feed in DirectX texture on the GPU
class CameraAPI
{
public:
  using Frame = std::optional<std::unique_ptr<Image2D>>;  
public:
  CameraAPI() = default;
  ~CameraAPI(){}
  virtual void Init() = 0; // TODO: think about the error handling
  virtual void Open(int index) = 0;
  bool IsOpened(){ return m_opened; }
  virtual Frame CaptureFrame() = 0;
  virtual void Close() = 0;
  int GetNumberOfDevices() {return m_numberOfDevices;}
  virtual std::string GetDeviceName(int index) = 0;
  std::optional<int> GetSelectedDevices() {
    if(m_opened)
      return m_selectedDevice;
    else
      return std::nullopt;
  }
protected:
  bool m_opened = false;
  int m_selectedDevice = -1;
  int m_numberOfDevices = 0;

};

} // namespace medicimage