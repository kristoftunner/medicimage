#pragma once
#include "renderer/texture.h"

#include <memory>
#include "optional"

namespace medicimage
{

// Simple camera interface, one thing is sure: we need the feed in DirectX texture on the GPU
class CameraAPI
{
public:
  using Frame = std::optional<std::unique_ptr<Texture2D>>;  
public:
  CameraAPI() = default;
  ~CameraAPI(){}
  virtual void Init() = 0; // TODO: think about the error handling
  virtual void Open() = 0;
  bool IsOpened(){ return m_opened; }
  virtual Frame CaptureFrame() = 0;
  virtual void Close() = 0;
protected:
  bool m_opened;
};

} // namespace medicimage
