#include "camera/opencv_camera.h"
#include "core/log.h"
#include "opencv2/core/directx.hpp"
#include "opencv2/core/ocl.hpp"
#include "opencv_camera.h"
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

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
    for (auto &device : devices)
    {
        device.release();
    }
}

void OpenCvCamera::Open(int index)
{
    assert(index < m_numberOfDevices && "Camera ID is out of range!");
    m_cap.open(index, cv::CAP_DSHOW);
    if (!m_cap.isOpened())
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
    if (!m_cap.isOpened())
    {
        APP_CORE_ERR("OpenCV camera capture closed unexpectedly!");
        return CameraAPI::Frame();
    }

    int width = 1280;
    int height = 720;
    cv::Mat frame;

    if (m_cap.read(frame))
    {
        cv::resize(frame, frame, cv::Size(width, height));
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);

        Frame frameTexture = std::make_unique<Image2D>(static_cast<unsigned char *>(frame.data), width, height, 3);

        return std::move(frameTexture);
    }
    else
        return CameraAPI::Frame();
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