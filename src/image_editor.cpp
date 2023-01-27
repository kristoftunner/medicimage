#include "image_editor.h"
#include "log.h"
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

  //APP_CORE_INFO("Dumped {%d}", mappedTexture.DepthPitch);
  Renderer::GetInstance().GetDeviceContext()->Unmap(texture, 0);
}

void ImageEditor::SetTextureForEditing(std::unique_ptr<Texture2D> texture)
{
  m_texture = std::move(texture);
  m_lines.clear();
  m_circles.clear();
  m_rectangles.clear();
  m_arrows.clear();

  m_tempLine.reset();
  m_tempCircle.reset();
  m_tempRectangle.reset();
  m_tempArrow.reset();
}

void ImageEditor::Init(ID3D11Device* device)
{
  if(cv::ocl::haveOpenCL())
    cv::directx::ocl::initializeContextFromD3D11Device(device);
  else
    APP_CORE_ERR("Do not have OpenCL device!!");
}

void ImageEditor::AddRectangle(ImVec2 topLeft, ImVec2 bottomRight, PrimitiveAddingType addType, const int thickness, const Color color)
{
  auto checkInput = [&](float input) { return ((input <= 1) && (input >= 0)); };
  if(!checkInput(topLeft.x) || !checkInput(topLeft.y) || !checkInput(bottomRight.x) || !checkInput(bottomRight.y) )
    return;
  
  int textureWidth = m_texture->GetWidth();
  int textureHeight = m_texture->GetHeight();
  cv::Point rectangleTopLeft = {static_cast<int>(topLeft.x * textureWidth), static_cast<int>(topLeft.y * textureHeight)}; 
  cv::Point rectangleBottomRight = {static_cast<int>(bottomRight.x * textureWidth), static_cast<int>(bottomRight.y * textureHeight)}; 
  
  if(addType == PrimitiveAddingType::PERMANENT)
  {
    m_rectangles.emplace_back(Rectangle{rectangleTopLeft, rectangleBottomRight, {thickness, color}});
    m_tempRectangle.reset();
  }
  else
    m_tempRectangle.emplace(Rectangle{rectangleTopLeft, rectangleBottomRight, {thickness, color}});  
  APP_CORE_TRACE("Added rectangle: topleft: {}:{} bottomRight:{}:{}", rectangleTopLeft.x, rectangleTopLeft.y, rectangleBottomRight.x, rectangleBottomRight.y);
}

void ImageEditor::AddCircle(ImVec2 center, float radius, PrimitiveAddingType addType, const int thickness, const Color color)
{
  auto checkInput = [&](float input) { return ((input <= 1) && (input >= 0)); };
  if(!checkInput(center.x) || !checkInput(center.y))
    return;
  
  int textureWidth = m_texture->GetWidth();
  int textureHeight = m_texture->GetHeight();
  cv::Point circleCenter = {static_cast<int>(center.x * textureWidth), static_cast<int>(center.y * textureHeight)}; 
  float circleRadius = radius * textureWidth; 
  if(addType == PrimitiveAddingType::PERMANENT)
  {
    m_circles.emplace_back(Circle{circleCenter, circleRadius, {thickness, color}}); // radius multiplied with texture widht for simplicity
    m_tempCircle.reset();
  }
  else
    m_tempCircle.emplace(Circle{circleCenter, circleRadius, {thickness, color}});
  APP_CORE_TRACE("Added circle: {}:{} r:{}", circleCenter.x, circleCenter.y, circleRadius);
}

void ImageEditor::AddLine(ImVec2 begin, ImVec2 end, PrimitiveAddingType addType, const int thickness, const Color color)
{
  auto checkInput = [&](float input) { return ((input <= 1) && (input >= 0)); };
  if(!checkInput(begin.x) || !checkInput(begin.y) || !checkInput(end.x) || !checkInput(end.y))
    return;
  
  int textureWidth = m_texture->GetWidth();
  int textureHeight = m_texture->GetHeight();
  cv::Point lineBegin = {static_cast<int>(begin.x * textureWidth), static_cast<int>(begin.y * textureHeight)}; 
  cv::Point lineEnd = {static_cast<int>(end.x * textureWidth), static_cast<int>(end.y * textureHeight)}; 
  
  if(addType == PrimitiveAddingType::PERMANENT)
  {
    m_lines.emplace_back(Line{lineBegin, lineEnd, {thickness, color}});
    m_tempLine.reset();
  }
  else
    m_tempLine.emplace(Line{lineBegin, lineEnd, {thickness, color}});
  APP_CORE_TRACE("Added a line: begin: {}:{} end:{}:{}", lineBegin.x, lineBegin.y, lineEnd.x, lineEnd.y);
}

void ImageEditor::AddArrow(ImVec2 begin, ImVec2 end, PrimitiveAddingType addType, const int thickness, const Color color)
{
  auto checkInput = [&](float input) { return ((input <= 1) && (input >= 0)); };
  if(!checkInput(begin.x) || !checkInput(begin.y) || !checkInput(end.x) || !checkInput(end.y))
    return;
  
  int textureWidth = m_texture->GetWidth();
  int textureHeight = m_texture->GetHeight();
  cv::Point lineBegin = {static_cast<int>(begin.x * textureWidth), static_cast<int>(begin.y * textureHeight)}; 
  cv::Point lineEnd = {static_cast<int>(end.x * textureWidth), static_cast<int>(end.y * textureHeight)}; 
  
  if(addType == PrimitiveAddingType::PERMANENT)
  {
    m_arrows.emplace_back(Line{lineBegin, lineEnd, {thickness, color}});
    m_tempArrow.reset();
  }
  else
    m_tempArrow.emplace(Line{lineBegin, lineEnd, {thickness, color}});
  APP_CORE_TRACE("Added a arrow: begin: {}:{} end:{}:{}", lineBegin.x, lineBegin.y, lineEnd.x, lineEnd.y);
}

void ImageEditor::AddText(const std::string& text, ImVec2 bottomLeft, PrimitiveAddingType addType,const int fontSize, const Color color)
{
  auto checkInput = [&](float input) { return ((input <= 1) && (input >= 0)); };
  if(!checkInput(bottomLeft.x) || !checkInput(bottomLeft.y))
    return;
  
  int textureWidth = m_texture->GetWidth();
  int textureHeight = m_texture->GetHeight();
  cv::Point bottomLeftCorner = {static_cast<int>(bottomLeft.x * textureWidth), static_cast<int>(bottomLeft.y * textureHeight)}; 
  
  if(addType == PrimitiveAddingType::PERMANENT)
  {
    m_texts.emplace_back(Text{text, bottomLeftCorner, fontSize, color});
    m_tempText.reset();
  }
  else
    m_tempText.emplace(Text{text, bottomLeftCorner, fontSize, color});
  APP_CORE_TRACE("Added text:{} at:{}:{}", text, bottomLeft.x, bottomLeft.y);
}

std::shared_ptr<Texture2D> ImageEditor::Draw()
{
  cv::directx::convertFromD3D11Texture2D(m_texture->GetTexturePtr(), m_opencvImage);
  cv::UMat image;
  cv::cvtColor(m_opencvImage, image, cv::COLOR_RGBA2BGR);
  for(auto& rectangle : m_rectangles)
  {
    const Color& color = rectangle.attributes.color;
    cv::rectangle(image, rectangle.topLeft, rectangle.bottomRight, cv::Scalar(color.blue,color.green,color.red), rectangle.attributes.thickness); // color has to be passed in BGR format 
  }
  if(m_tempRectangle)
  {
    const Color& color = m_tempRectangle.value().attributes.color;
    cv::rectangle(image, m_tempRectangle.value().topLeft, m_tempRectangle.value().bottomRight, cv::Scalar(color.blue,color.green,color.red), m_tempRectangle.value().attributes.thickness);
  }

  for(auto& circle : m_circles)
  {
    const Color& color = circle.attributes.color;
    cv::circle(image, circle.center, static_cast<int>(circle.radius), cv::Scalar(color.blue,color.green,color.red), circle.attributes.thickness); 
  }
  if(m_tempCircle)
  {
    const Color& color = m_tempCircle.value().attributes.color;
    cv::circle(image, m_tempCircle.value().center, static_cast<int>(m_tempCircle.value().radius), cv::Scalar(color.blue,color.green,color.red), m_tempCircle.value().attributes.thickness);
  }

  for(auto& line : m_lines)
  {
    const Color& color = line.attributes.color;
    cv::line(image, line.begin, line.end, cv::Scalar(color.blue,color.green,color.red), line.attributes.thickness);
  }
  if(m_tempLine)
  {
    const Color& color = m_tempLine.value().attributes.color;
    cv::line(image, m_tempLine.value().begin, m_tempLine.value().end, cv::Scalar(color.blue,color.green,color.red), m_tempLine.value().attributes.thickness);
  }

  for(auto& arrow : m_arrows)
  {
    const Color& color = arrow.attributes.color;
    cv::arrowedLine(image, arrow.begin, arrow.end, cv::Scalar(color.blue,color.green,color.red), arrow.attributes.thickness);
  }
  if(m_tempArrow)
  {
    const Color& color = m_tempArrow.value().attributes.color;
    cv::arrowedLine(image, m_tempArrow.value().begin, m_tempArrow.value().end, cv::Scalar(color.blue,color.green,color.red), m_tempArrow.value().attributes.thickness);
  }
  
  for(auto& text : m_texts)
  {
    const Color& color = text.color;
    cv::putText(image, text.text, text.bottomLeft, cv::FONT_HERSHEY_PLAIN, text.fontSize, cv::Scalar(color.blue,color.green,color.red), 3);
  }
  if(m_tempText)
  {
    const Color& color = m_tempText.value().color;
    cv::putText(image, m_tempText.value().text, m_tempText.value().bottomLeft, cv::FONT_HERSHEY_PLAIN, m_tempText.value().fontSize, cv::Scalar(color.blue, color.green, color.red), 3);
  }

  std::shared_ptr<Texture2D> dstTexture = std::make_shared<Texture2D>(m_texture->GetTexturePtr(), m_texture->GetName());
  cv::cvtColor(image, m_opencvImage, cv::COLOR_BGR2RGBA);
  cv::directx::convertToD3D11Texture2D(m_opencvImage, dstTexture->GetTexturePtr());
  return dstTexture;
}

std::shared_ptr<Texture2D> ImageEditor::AddImageFooter(const std::string& watermark, std::shared_ptr<Texture2D> texture)
{
  cv::UMat image;
  cv::directx::convertFromD3D11Texture2D(texture->GetTexturePtr(), image);
  cv::cvtColor(image, image, cv::COLOR_RGBA2BGR);

  // add a sticker to the bottom with the image name, date and time
  // assuming the original texture has 1920x1080 resolution, expanding with 20-20 pixels left/right and 30 bottom, 20 top
  constexpr int sideBorder = 20;
  constexpr int topBorder = 20;
  constexpr int bottomBorder = 100;
  cv::UMat borderedImage;
  cv::copyMakeBorder(image, borderedImage, topBorder, bottomBorder, sideBorder, sideBorder, cv::BORDER_CONSTANT , cv::Scalar{255,255,255} ); // adding white border
  cv::putText(borderedImage, watermark, cv::Point{20, borderedImage.rows - 20}, cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar{128,128,128}, 3);
  
  std::shared_ptr<Texture2D> dstTexture = std::make_shared<Texture2D>(borderedImage.cols, borderedImage.rows);
  dstTexture->SetName(texture->GetName());
  cv::cvtColor(borderedImage, borderedImage, cv::COLOR_BGR2RGBA);
  cv::directx::convertToD3D11Texture2D(borderedImage, dstTexture->GetTexturePtr());
  return dstTexture;
}

} // namespace medicimage
