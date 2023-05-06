#include "image_handling/image_editor.h"
#include "core/log.h"
#include <fstream>
#include "image_editor.h"
namespace medicimage
{
std::unique_ptr<Image2D> ImageEditor::s_image;
std::unique_ptr<wxMemoryDC> ImageEditor::s_dc;
//cv::UMat ImageEditor::s_image;
//static void DumpTexture(ID3D11Texture2D* texture)
//{
//  // create texture for copy back data from GPU
//  D3D11_TEXTURE2D_DESC desc;
//  ID3D11Texture2D *readbackTexture;
//  texture->GetDesc(&desc);
//  desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
//  desc.Usage = D3D11_USAGE_STAGING;
//  desc.MiscFlags = 0;
//  desc.BindFlags = 0;
//  ThrowIfFailed(Renderer::GetInstance().GetDevice()->CreateTexture2D(&desc, 0, &readbackTexture));
//
//  // read back the data from GPU to CPU
//  Renderer::GetInstance().GetDeviceContext()->CopyResource(readbackTexture, texture);
//
//  D3D11_MAPPED_SUBRESOURCE mappedTexture;
//  int subresource = ::D3D11CalcSubresource(0,0,1);  
//  ThrowIfFailed(Renderer::GetInstance().GetDeviceContext()->Map(readbackTexture, subresource, D3D11_MAP_READ, 0, &mappedTexture));
//  std::ofstream fileStream("filename", std::ios::binary);
//  fileStream.write(reinterpret_cast<char*>(mappedTexture.pData), mappedTexture.DepthPitch);
//
//  Renderer::GetInstance().GetDeviceContext()->Unmap(texture, 0);
//}

//void ImageEditor::Init(ID3D11Device* device)
//{
//  std::vector<cv::ocl::PlatformInfo> platformInfos;
//  auto convertDeviceType = [](int type) {
//    std::string deviceType;
//    switch (type)
//    {
//    case 0:
//      deviceType = "DEFAULT";
//      break;
//    case 2:
//      deviceType = "CPU";
//      break;
//    case 4:
//      deviceType = "GPU";
//      break;
//    case (4 + (1<<16)):
//      deviceType = "IGPU";
//      break;
//    default:
//      deviceType = "some weird vendor type";
//      break;
//    }
//    return deviceType;
//  };
//  if(cv::ocl::haveOpenCL())
//  {
//    cv::ocl::getPlatfomsInfo(platformInfos);
//    for(auto& platform : platformInfos)
//    {
//      cv::ocl::Device device;
//      platform.getDevice(device, 0); // assuming there is one device per platform
//      auto type = device.type();
//      auto vendorId = device.vendorID();
//      auto name = platform.name();
//      auto vendor = platform.vendor();
//      std::string deviceType = convertDeviceType(type);
//      APP_CORE_INFO("Available openCL device: {} type:{} vendor: {} vendorID: {}", name, deviceType, vendor, vendorId);
//    }
//
//    m_context = cv::directx::ocl::initializeContextFromD3D11Device(device);
//
//    std::string deviceName = m_context.device(0).name();
//    auto vendorId = m_context.device(0).vendorID();
//    auto vendorName = m_context.device(0).vendorName();
//    auto type = m_context.device(0).type();
//    auto deviceType = convertDeviceType(type);
//    APP_CORE_INFO("Using the following device: {} type:{} vendor: {} vendorID: {}", deviceName, deviceType, vendorName, vendorId);
//  }
//  else
//    APP_CORE_ERR("Do not have OpenCL device!!");
//}

void ImageEditor::Begin(std::unique_ptr<Image2D> image)
{
  s_image = std::move(image);
  s_dc = std::make_unique<wxMemoryDC>(s_image->GetBitmap());
}

std::unique_ptr<Image2D> ImageEditor::End()
{
  s_dc->SelectObject(wxNullBitmap);
  s_dc.reset();
  return std::move(s_image);
}


void ImageEditor::DrawCircle(glm::vec2 center, float radius, glm::vec4 color, float thickness, bool filled)
{
  glm::vec2 imageSize = {s_image->GetWidth(), s_image->GetHeight()};
  center *= imageSize;
  radius = radius * glm::length(imageSize);
  auto alpha = color.a;
  color *= 255.0;
  if(filled)
  { // TODO add transparency support with wxGraphicsContext
    //s_image.copyTo(overlay);
    //cv::circle(s_image, cv::Point{static_cast<int>(center.x), static_cast<int>(center.y)}, static_cast<int>(radius), cv::Scalar(color.b, color.g,  color.r), -1); 
    //cv::addWeighted(overlay, alpha, s_image, 1 - alpha, 0, s_image);
  }
  else
    s_dc->DrawCircle(wxPoint{static_cast<int>(center.x), static_cast<int>(center.y)}, static_cast<int>(radius));

  //TODO: add rotation 
}

void ImageEditor::DrawRectangle(glm::vec2 topleft, glm::vec2 bottomright, glm::vec4 color, float thickness, bool filled)
{
  glm::vec2 imageSize = {s_image->GetWidth(), s_image->GetHeight()};
  topleft *= imageSize;
  bottomright *= imageSize; 
  
  if ((static_cast<int>(topleft.x) != static_cast<int>(bottomright.x)) && (static_cast<int>(topleft.y) != static_cast<int>(bottomright.y)))
  {
    float alpha = color.a;
    color *= 255.0;
    if(filled)
    {
      //s_image.copyTo(overlay);
      //cv::rectangle(overlay, cv::Point{ static_cast<int>(topleft.x), static_cast<int>(topleft.y) }, cv::Point{ static_cast<int>(bottomright.x), static_cast<int>(bottomright.y) },
      //  cv::Scalar(color.b, color.g, color.r), -1);
      //cv::addWeighted(overlay, alpha, s_image, 1 - alpha, 0, s_image);
    }
    else
      s_dc->DrawRectangle(wxRect(wxPoint{static_cast<int>(topleft.x), static_cast<int>(topleft.y)}, wxPoint{static_cast<int>(bottomright.x), static_cast<int>(bottomright.y)}));
      //cv::rectangle(s_image, cv::Point{ static_cast<int>(topleft.x), static_cast<int>(topleft.y) }, cv::Point{ static_cast<int>(bottomright.x), static_cast<int>(bottomright.y) },
      //  cv::Scalar(color.b, color.g, color.r), static_cast<int>(thickness), cv::LineTypes::FILLED);
  }

  //TODO: add rotation 
}

void ImageEditor::DrawArrow(glm::vec2 begin, glm::vec2 end, glm::vec4 color, float thickness, double tipLength)
{
  glm::vec2 imageSize = {s_image->GetWidth(), s_image->GetHeight()};
  begin *= imageSize; 
  end *= imageSize; 
  auto scaledLength = 10 / glm::length(end - begin);
  color *= 255.0;

  s_dc->DrawLine(wxPoint{static_cast<int>(begin.x), static_cast<int>(begin.y)}, wxPoint{static_cast<int>(end.x), static_cast<int>(end.y)}); // TODO: add arrow cuz it is only a line
  //cv::arrowedLine(s_image, cv::Point(static_cast<int>(begin.x), static_cast<int>(begin.y)), cv::Point(static_cast<int>(end.x), static_cast<int>(end.y)), 
  //  cv::Scalar(color.b, color.g, color.r), static_cast<int>(thickness), tipLength = scaledLength);

  //TODO: add rotation 
}
void ImageEditor::DrawLine(glm::vec2 begin, glm::vec2 end, glm::vec4 color, float thickness, double tipLengith)
{
  glm::vec2 imageSize = {s_image->GetWidth(), s_image->GetHeight()};
  begin *= imageSize; 
  end *= imageSize; 
  color *= 255.0;

  s_dc->DrawLine(wxPoint{static_cast<int>(begin.x), static_cast<int>(begin.y)}, wxPoint{static_cast<int>(end.x), static_cast<int>(end.y)}); // TODO: add arrow cuz it is only a line
  //cv::line(s_image, cv::Point(static_cast<int>(begin.x), static_cast<int>(begin.y)), cv::Point(static_cast<int>(end.x), static_cast<int>(end.y)), 
  //  cv::Scalar(color.b, color.g, color.r), static_cast<int>(thickness));
}

void ImageEditor::DrawText(glm::vec2 bottomLeft, const std::string &text, int fontSize, float thickness)
{
  // TODO: do nothing for now
  //int baseline = 0;
  //glm::vec2 imageSize = {s_image->GetWidht(), s_image->GetHeight()};
  //cv::Point scaledBottomLeft{ static_cast<int>(bottomLeft.x * imageSize.x), static_cast<int>(bottomLeft.y * imageSize.y) };
  //constexpr auto backgroundScaler = 1.0;
  //auto textSize = cv::getTextSize(text, s_defaultFont, fontSize * backgroundScaler, thickness, &baseline);
  //auto bgTopRight = scaledBottomLeft + cv::Point{ textSize.width, -textSize.height };
  //auto bgBottomLeft = scaledBottomLeft + cv::Point(0, baseline * backgroundScaler);
  //cv::rectangle(s_image, cv::Rect(bgBottomLeft, bgTopRight), cv::Scalar::all(255), -1); // white rectangle behind the text
  //cv::putText(s_image, text, scaledBottomLeft, s_defaultFont, fontSize, cv::Scalar::all(0), thickness);
}


void ImageEditor::DrawSpline(glm::vec2 begin, glm::vec2 middle, glm::vec2 end, int lineCount, glm::vec4 color, float thickness)
{
  // TODO: do nothing for now
  //std::vector<glm::vec2> splinePoints;
  //const float diff = 1.0 / static_cast<float>(lineCount);
  //glm::vec2 imageSize = {s_image.cols, s_image.rows};
  //for(int i = 0; i < (lineCount - 1); i++)
  //{
  //  glm::vec2 t(diff * (i + 1)); 
  //  glm::vec2 bezierPoint = t * t * begin + glm::vec2(2) * t * (glm::vec2(1) - t) * middle + (glm::vec2(1) - t)*(glm::vec2(1) - t) * end;
  //  bezierPoint *= imageSize;
  //  splinePoints.push_back(bezierPoint);
  //}

  //auto scaledColor = color * glm::vec4(255.0);
  //for(int i = 0; i < splinePoints.size() - 1; i++)
  //{
  //  auto begin = splinePoints[i];
  //  auto end = splinePoints[i + 1];
  //  cv::line(s_image, cv::Point(static_cast<int>(begin.x), static_cast<int>(begin.y)), cv::Point(static_cast<int>(end.x), static_cast<int>(end.y)), 
  //    cv::Scalar(scaledColor.b, scaledColor.g, scaledColor.r), static_cast<int>(thickness));
  //}
}

glm::vec2 ImageEditor::GetTextBoundingBox(const std::string &text, int fontSize, float thickness)
{
  int baseline = 0;
  auto textSize = cv::getTextSize(text, s_defaultFont, fontSize, thickness, &baseline); 
  return glm::vec2{static_cast<float>(textSize.width) / static_cast<float>(s_image->GetWidth()), static_cast<float>(textSize.height) / static_cast<float>(s_image->GetHeight())};
}

Image2D ImageEditor::AddFooter(Image2D& image, const std::string& footerText)
{
  // add a sticker to the bottom with the image name, date and time
  // assuming the original texture has 1920x1080 resolution, expanding with 20-20 pixels left/right and 30 bottom, 20 top
  // TODO: do nothing for now

  //cv::UMat borderedImage;
  //cv::copyMakeBorder(image, borderedImage, s_topBorder, s_bottomBorder, s_sideBorder, s_sideBorder, cv::BORDER_CONSTANT , cv::Scalar{255,255,255} ); // adding white border
  //cv::putText(borderedImage, footerText, cv::Point{s_topBorder, borderedImage.rows - s_topBorder}, s_defaultFont, 1, cv::Scalar{0,0,0}, 3);
  //return borderedImage;
  return image;
}

std::unique_ptr<Image2D> ImageEditor::ReplaceImageFooter(const std::string& footerText, Image2D& image)
{
  //cv::UMat image;
  //cv::directx::convertFromD3D11Texture2D(texture->GetTexturePtr(), image);
  //image = image(cv::Range(s_topBorder, image.rows - s_bottomBorder), cv::Range(s_sideBorder, image.cols - s_sideBorder));
  //cv::cvtColor(image, image, cv::COLOR_RGBA2BGR);
  
  //cv::UMat borderedImage = AddFooter(image, footerText);
  
  //std::unique_ptr<Image2D> dstTexture = std::make_unique<Image2D>(texture->GetName(), borderedImage.cols, borderedImage.rows);
  //dstTexture->SetName(texture->GetName());
  //cv::cvtColor(borderedImage, borderedImage, cv::COLOR_BGR2RGBA);
  //cv::directx::convertToD3D11Texture2D(borderedImage, dstTexture->GetTexturePtr());
  //return std::move(dstTexture);
  return std::make_unique<Image2D>(image);
}

std::unique_ptr<Image2D> ImageEditor::RemoveFooter(Image2D& image)
{
  //cv::UMat image;
  //cv::directx::convertFromD3D11Texture2D(texture->GetTexturePtr(), image);
  //image = image(cv::Range(s_topBorder, image.rows - s_bottomBorder), cv::Range(s_sideBorder, image.cols - s_sideBorder));
  //cv::cvtColor(image, image, cv::COLOR_RGBA2BGR);
  //cv::cvtColor(image, image, cv::COLOR_BGR2RGBA);
  
  //std::unique_ptr<Image2D> dstTexture = std::make_unique<Image2D>(texture->GetName(), image.cols, image.rows);
  //dstTexture->SetName(texture->GetName());
  //cv::directx::convertToD3D11Texture2D(image, dstTexture->GetTexturePtr());
  //return std::move(dstTexture);
  return std::make_unique<Image2D>(image);
}

std::unique_ptr<Image2D> ImageEditor::AddImageFooter(const std::string& footerText, Image2D& image)
{
  //cv::UMat image;
  //cv::directx::convertFromD3D11Texture2D(texture->GetTexturePtr(), image);
  //cv::cvtColor(image, image, cv::COLOR_RGBA2BGR);
  
  //cv::UMat borderedImage = AddFooter(image, footerText);
  
  //std::unique_ptr<Image2D> dstTexture = std::make_unique<Image2D>(texture->GetName(), borderedImage.cols, borderedImage.rows);
  //dstTexture->SetName(texture->GetName());
  //cv::cvtColor(borderedImage, borderedImage, cv::COLOR_BGR2RGBA);
  //cv::directx::convertToD3D11Texture2D(borderedImage, dstTexture->GetTexturePtr());
  //return std::move(dstTexture);
  return std::make_unique<Image2D>(image);
}

} // namespace medicimage
