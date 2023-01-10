#pragma once

#include "d3d11.h"

namespace medicimage
{

// Simple camera interface, one thing is sure: we need the feed in DirectX texture on the GPU
class CameraAPI
{
public:
  CameraAPI() = default;
  ~CameraAPI(){}
  virtual void Init() {}; // TODO: think about the error handling
  virtual void Open() {};
  bool IsOpened(){ return m_opened; }
  virtual void ReadFrame() {};
  virtual void Close() {};
protected:
  bool m_opened;
};

} // namespace medicimage
