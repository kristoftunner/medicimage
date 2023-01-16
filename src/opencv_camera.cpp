#include "opencv_camera.h"
#include "log.h"
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

  int width  = static_cast<int>(m_cap.get(cv::CAP_PROP_FRAME_WIDTH));
  int height = static_cast<int>(m_cap.get(cv::CAP_PROP_FRAME_HEIGHT));
  int format = static_cast<int>(m_cap.get(cv::CAP_PROP_FORMAT));
  
  cv::Mat frame;
  m_cap.read(frame);
  cv::cvtColor(frame, frame, cv::COLOR_BGR2RGBA);
  Frame frameTexture = std::make_unique<Texture2D>(width, height);
  Renderer::GetInstance().GetDeviceContext()->UpdateSubresource(frameTexture.value()->GetTexturePtr(), 0, 0, frame.data, (UINT)frame.step[0], (UINT)frame.total());

  return std::move(frameTexture);
}

void OpenCvCamera::Close()
{
  m_cap.release();
}
  
} // namespace medicimage
