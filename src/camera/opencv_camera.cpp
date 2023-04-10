#include "camera/opencv_camera.h"
#include "core/log.h"
#include "opencv2/core/directx.hpp"
#include "opencv2/core/ocl.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

namespace medicimage
{

OpenCvCamera::OpenCvCamera(int cameraId) : m_cameraId(cameraId), CameraAPI()
{
  
}

void OpenCvCamera::Open()
{
  m_cap.open(m_cameraId, cv::CAP_ANY);
  if(!m_cap.isOpened())
    APP_CORE_ERR("Cannot open camera!!");
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
  //cv::resize(frame, frame, cv::Size(width, height));
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
  
} // namespace medicimage
