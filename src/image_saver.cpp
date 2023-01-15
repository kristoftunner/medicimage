#include "image_saver.h"
#include "log.h"

#include "opencv2/core/directx.hpp"
#include "opencv2/core/ocl.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

namespace medicimage
{
ImageSaver::ImageSaver(int uuid) : m_uuid(uuid)
{
  m_dirPath = "data";
  m_dirPath /= std::to_string(uuid);

  if(!(std::filesystem::is_directory(m_dirPath)));
    if(!(std::filesystem::create_directory(m_dirPath)))
      APP_CORE_ERR("Cannot create directory"); 
}

void ImageSaver::SaveImage(std::shared_ptr<Texture2D> texture)
{
  cv::UMat image;
  cv::directx::convertFromD3D11Texture2D(texture->GetTexturePtr(), image);
  cv::cvtColor(image, image, cv::COLOR_RGBA2BGR);
  std::string name = std::to_string(m_uuid) + "_" + std::to_string(m_savedImages.size());
  texture->SetName(name);
  
  name += ".jpeg";
  std::filesystem::path path = m_dirPath;
  path /= name; 
  cv::imwrite(path.string(), image);
}

void ImageSaver::DeleteImage(const std::string& imageName)
{
  
} 
  
} // namespace medicimage
