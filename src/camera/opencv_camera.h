#pragma once

#include "camera/camera_api.h"

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

namespace medicimage
{

class OpenCvCamera final : public CameraAPI
{
public:
  ~OpenCvCamera() = default;
  void Init() override;  
  void Open(int index) override;
  CameraAPI::Frame CaptureFrame() override;
  void Close() override;
  std::string GetDeviceName(int index) override;
private:
  cv::VideoCapture m_cap;
  std::vector<std::string> m_deviceNames;
};
 
} // namespace medicimage
