#include "camera_api.h"

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

namespace medicimage
{

class OpenCvCamera final : public CameraAPI
{
public:
  OpenCvCamera(int cameraId);
  ~OpenCvCamera() = default;
  void Init() override {} // there is nothing to initialize with OpenCV camera capture api, just need to open
  void Open() override;
  CameraAPI::Frame CaptureFrame() override;
  void Close() override;
private:
  int m_cameraId = 0;
  cv::VideoCapture m_cap;
};
 
} // namespace medicimage
