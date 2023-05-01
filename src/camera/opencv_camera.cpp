#include "camera/opencv_camera.h"
#include "core/log.h"
#include "opencv2/core/directx.hpp"
#include "opencv2/core/ocl.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include "opencv_camera.h"

namespace medicimage
{

void OpenCvCamera::Init()
{
  std::vector<cv::VideoCapture> devices;
  cv::VideoCapture temp;

  // Iterate through all device IDs starting from 0 until no more devices are found
  for (int i = 0;; i++)
  {
    temp.open(i, cv::CAP_DSHOW);
    if (!temp.isOpened())
      break;
    std::string deviceName = std::string("Camera") + std::to_string(i);
    m_deviceNames.push_back(deviceName);
    devices.push_back(temp);
  }

  m_numberOfDevices = devices.size();
  APP_CORE_INFO("Number of cameras attached: {0}", m_numberOfDevices);
  for(auto& device : devices)
  {
    device.release();
  }
}

void OpenCvCamera::Open(int index)
{
  assert(index < m_numberOfDevices && "Camera ID is out of range!");
  m_cap.open(index, cv::CAP_DSHOW);
  if(!m_cap.isOpened())
  {
    m_selectedDevice = -1;
    m_opened = false;
    APP_CORE_ERR("Cannot open camera!!");
  }
  else
  {
    m_opened = true;
    m_selectedDevice = index;
  }
}

CameraAPI::Frame OpenCvCamera::CaptureFrame()
{
  if(!m_cap.isOpened())
  {
    APP_CORE_ERR("OpenCV camera capture closed unexpectedly!");
    return CameraAPI::Frame();
  }

  constexpr int width = 1920;
  constexpr int height = 1080; 
  cv::UMat frame;
  m_cap.read(frame);
  cv::resize(frame, frame, cv::Size(width, height));
  cv::cvtColor(frame, frame, cv::COLOR_BGR2RGBA);
  
  Frame frameTexture = std::make_unique<Texture2D>("frame", frame.cols, frame.rows);
  cv::directx::convertToD3D11Texture2D(frame, frameTexture.value()->GetTexturePtr());
  frame.release();
  return std::move(frameTexture);
}

void OpenCvCamera::Close()
{
  m_cap.release();
}
std::string OpenCvCamera::GetDeviceName(int index)
{
  return m_deviceNames[index];
}
} // namespace medicimage
