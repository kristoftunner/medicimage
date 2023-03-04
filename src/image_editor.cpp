#include "image_editor.h"
#include "core/log.h"
#include <fstream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

namespace medicimage
{

static void DumpTexture(ID3D11Texture2D* texture)
{
  // create texture for copy back data from GPU
  D3D11_TEXTURE2D_DESC desc;
  ID3D11Texture2D *readbackTexture;
  texture->GetDesc(&desc);
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
  desc.Usage = D3D11_USAGE_STAGING;
  desc.MiscFlags = 0;
  desc.BindFlags = 0;
  ThrowIfFailed(Renderer::GetInstance().GetDevice()->CreateTexture2D(&desc, 0, &readbackTexture));

  // read back the data from GPU to CPU
  Renderer::GetInstance().GetDeviceContext()->CopyResource(readbackTexture, texture);

  D3D11_MAPPED_SUBRESOURCE mappedTexture;
  int subresource = ::D3D11CalcSubresource(0,0,1);  
  ThrowIfFailed(Renderer::GetInstance().GetDeviceContext()->Map(readbackTexture, subresource, D3D11_MAP_READ, 0, &mappedTexture));
  std::ofstream fileStream("filename", std::ios::binary);
  fileStream.write(reinterpret_cast<char*>(mappedTexture.pData), mappedTexture.DepthPitch);

  Renderer::GetInstance().GetDeviceContext()->Unmap(texture, 0);
}

void ImageEditor::Init(ID3D11Device* device)
{
  std::vector<cv::ocl::PlatformInfo> platformInfos;
  auto convertDeviceType = [](int type) {
    std::string deviceType;
    switch (type)
    {
    case 0:
      deviceType = "DEFAULT";
      break;
    case 2:
      deviceType = "CPU";
      break;
    case 4:
      deviceType = "GPU";
      break;
    case (4 + (1<<16)):
      deviceType = "IGPU";
      break;
    default:
      deviceType = "some weird vendor type";
      break;
    }
    return deviceType;
  };
  if(cv::ocl::haveOpenCL())
  {
    cv::ocl::getPlatfomsInfo(platformInfos);
    for(auto& platform : platformInfos)
    {
      cv::ocl::Device device;
      platform.getDevice(device, 0); // assuming there is one device per platform
      auto type = device.type();
      auto vendorId = device.vendorID();
      auto name = platform.name();
      auto vendor = platform.vendor();
      std::string deviceType = convertDeviceType(type);
      APP_CORE_INFO("Available openCL device: {} type:{} vendor: {} vendorID: {}", name, deviceType, vendor, vendorId);
    }

    m_context = cv::directx::ocl::initializeContextFromD3D11Device(device);

    std::string deviceName = m_context.device(0).name();
    auto vendorId = m_context.device(0).vendorID();
    auto vendorName = m_context.device(0).vendorName();
    auto type = m_context.device(0).type();
    auto deviceType = convertDeviceType(type);
    APP_CORE_INFO("Using the following device: {} type:{} vendor: {} vendorID: {}", deviceName, deviceType, vendorName, vendorId);
  }
  else
    APP_CORE_ERR("Do not have OpenCL device!!");
}

void ImageEditor::DrawCircle(Texture2D* texture, glm::vec2 center, float radius, glm::vec4 color, float thickness)
{
  cv::UMat image;
  cv::directx::convertFromD3D11Texture2D(texture->GetTexturePtr(), image);
  cv::cvtColor(image, image, cv::COLOR_RGBA2BGR);

  glm::vec2 imageSize = {texture->GetWidth(), texture->GetHeight()};
  center *= imageSize;
  radius = radius * glm::length(imageSize);
  color *= 255.0;
  cv::circle(image, cv::Point{static_cast<int>(center.x), static_cast<int>(center.y)}, static_cast<int>(radius), cv::Scalar(color.b, color.g,  color.r), thickness); 

  //TODO: add rotation 
  cv::cvtColor(image, image, cv::COLOR_BGR2RGBA);
  cv::directx::convertToD3D11Texture2D(image, texture->GetTexturePtr());
}

void ImageEditor::DrawRectangle(Texture2D* texture, glm::vec2 topleft, glm::vec2 bottomright, glm::vec4 color, float thickness)
{
  cv::UMat image;
  cv::directx::convertFromD3D11Texture2D(texture->GetTexturePtr(), image);
  cv::cvtColor(image, image, cv::COLOR_RGBA2BGR);

  glm::vec2 imageSize = {texture->GetWidth(), texture->GetHeight()};
  topleft *= imageSize;
  bottomright *= imageSize; 
  color *= 255.0;
  
  if ((static_cast<int>(topleft.x) != static_cast<int>(bottomright.x)) && (static_cast<int>(topleft.y) != static_cast<int>(bottomright.y)))
  {
    cv::rectangle(image, cv::Point{ static_cast<int>(topleft.x), static_cast<int>(topleft.y) }, cv::Point{ static_cast<int>(bottomright.x), static_cast<int>(bottomright.y) },
      cv::Scalar(color.b, color.g, color.r), static_cast<int>(thickness), cv::LineTypes::FILLED);
  }

  //TODO: add rotation 
  cv::cvtColor(image, image, cv::COLOR_BGR2RGBA);
  cv::directx::convertToD3D11Texture2D(image, texture->GetTexturePtr()); 
}

void ImageEditor::DrawArrow(Texture2D* texture, glm::vec2 begin, glm::vec2 end, glm::vec4 color, float thickness, double tipLength)
{
  cv::UMat image;
  cv::directx::convertFromD3D11Texture2D(texture->GetTexturePtr(), image);
  cv::cvtColor(image, image, cv::COLOR_RGBA2BGR);

  glm::vec2 imageSize = {texture->GetWidth(), texture->GetHeight()};
  begin *= imageSize; 
  end *= imageSize; 
  color *= 255.0;

  cv::arrowedLine(image, cv::Point(static_cast<int>(begin.x), static_cast<int>(begin.y)), cv::Point(static_cast<int>(end.x), static_cast<int>(end.y)), 
    cv::Scalar(color.b, color.g, color.r), static_cast<int>(thickness));

  //TODO: add rotation 
  cv::cvtColor(image, image, cv::COLOR_BGR2RGBA);
  cv::directx::convertToD3D11Texture2D(image, texture->GetTexturePtr());
}
cv::UMat ImageEditor::AddFooter(cv::UMat image, const std::string& footerText)
{
  // add a sticker to the bottom with the image name, date and time
  // assuming the original texture has 1920x1080 resolution, expanding with 20-20 pixels left/right and 30 bottom, 20 top
  cv::UMat borderedImage;
  cv::copyMakeBorder(image, borderedImage, s_topBorder, s_bottomBorder, s_sideBorder, s_sideBorder, cv::BORDER_CONSTANT , cv::Scalar{255,255,255} ); // adding white border
  cv::putText(borderedImage, footerText, cv::Point{20, borderedImage.rows - 20}, cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar{128,128,128}, 3);
  return borderedImage;
}

std::unique_ptr<Texture2D> ImageEditor::ReplaceImageFooter(const std::string& footerText, Texture2D* texture)
{
  cv::UMat image;
  cv::directx::convertFromD3D11Texture2D(texture->GetTexturePtr(), image);
  image = image(cv::Range(s_topBorder, image.rows - s_topBorder - s_bottomBorder), cv::Range(s_sideBorder, image.cols - 2*s_sideBorder));
  cv::cvtColor(image, image, cv::COLOR_RGBA2BGR);
  
  cv::UMat borderedImage = AddFooter(image, footerText);
  
  std::unique_ptr<Texture2D> dstTexture = std::make_unique<Texture2D>(texture->GetName(), borderedImage.cols, borderedImage.rows);
  dstTexture->SetName(texture->GetName());
  cv::cvtColor(borderedImage, borderedImage, cv::COLOR_BGR2RGBA);
  cv::directx::convertToD3D11Texture2D(borderedImage, dstTexture->GetTexturePtr());
  return std::move(dstTexture);
}

std::unique_ptr<Texture2D> ImageEditor::AddImageFooter(const std::string& footerText, Texture2D* texture)
{
  cv::UMat image;
  cv::directx::convertFromD3D11Texture2D(texture->GetTexturePtr(), image);
  cv::cvtColor(image, image, cv::COLOR_RGBA2BGR);
  
  cv::UMat borderedImage = AddFooter(image, footerText);
  
  std::unique_ptr<Texture2D> dstTexture = std::make_unique<Texture2D>(texture->GetName(), borderedImage.cols, borderedImage.rows);
  dstTexture->SetName(texture->GetName());
  cv::cvtColor(borderedImage, borderedImage, cv::COLOR_BGR2RGBA);
  cv::directx::convertToD3D11Texture2D(borderedImage, dstTexture->GetTexturePtr());
  return std::move(dstTexture);
}

} // namespace medicimage
