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
  cv::directx::convertFromD3D11Texture2D(m_texture->GetTexturePtr(), m_opencvImage);
  m_lines.clear();
  m_circles.clear();
  m_rectangles.clear();
  m_arrows.clear();
}

void ImageEditor::Init(ID3D11Device* device)
{
  if(cv::ocl::haveOpenCL())
    cv::directx::ocl::initializeContextFromD3D11Device(device);
  else
    APP_CORE_ERR("Do not have OpenCL device!!");
}

void ImageEditor::AddRectangle(ImVec2 topLeft, ImVec2 bottomRight)
{
  auto checkInput = [&](float input) { return ((input <= 1) && (input >= 0)); };
  if(!checkInput(topLeft.x) || !checkInput(topLeft.y) || !checkInput(bottomRight.x) || !checkInput(bottomRight.y) )
    return;
  
  int textureWidth = m_texture->GetWidth();
  int textureHeight = m_texture->GetHeight();
  cv::Point rectangleTopLeft = {static_cast<int>(topLeft.x * textureWidth), static_cast<int>(topLeft.y * textureHeight)}; 
  cv::Point rectangleBottomRight = {static_cast<int>(bottomRight.x * textureWidth), static_cast<int>(bottomRight.y * textureHeight)}; 
  m_rectangles.push_back(Rectangle{rectangleTopLeft, rectangleBottomRight});
  APP_CORE_TRACE("Added rectangle: topleft: {}:{} bottomRight:{}:{}", rectangleTopLeft.x, rectangleTopLeft.y, rectangleBottomRight.x, rectangleBottomRight.y);
}

void ImageEditor::AddCircle(ImVec2 center, float radius)
{
  auto checkInput = [&](float input) { return ((input <= 1) && (input >= 0)); };
  if(!checkInput(center.x) || !checkInput(center.y))
    return;
  
  int textureWidth = m_texture->GetWidth();
  int textureHeight = m_texture->GetHeight();
  cv::Point circleCenter = {static_cast<int>(center.x * textureWidth), static_cast<int>(center.y * textureHeight)}; 
  m_circles.push_back(Circle{circleCenter, radius * textureWidth}); // radius multiplied with texture widht for simplicity
  APP_CORE_TRACE("Added circle: {}:{} r:{}", circleCenter.x, circleCenter.y, radius*textureWidth);
}

void ImageEditor::AddLine(ImVec2 begin, ImVec2 end)
{
  auto checkInput = [&](float input) { return ((input <= 1) && (input >= 0)); };
  if(!checkInput(begin.x) || !checkInput(begin.y) || !checkInput(end.x) || !checkInput(end.y))
    return;
  
  int textureWidth = m_texture->GetWidth();
  int textureHeight = m_texture->GetHeight();
  cv::Point lineBegin = {static_cast<int>(begin.x * textureWidth), static_cast<int>(begin.y * textureHeight)}; 
  cv::Point lineEnd = {static_cast<int>(end.x * textureWidth), static_cast<int>(end.y * textureHeight)}; 
  m_lines.push_back(Line{lineBegin, lineEnd});
  APP_CORE_TRACE("Added a line: begin: {}:{} end:{}:{}", lineBegin.x, lineBegin.y, lineEnd.x, lineEnd.y);
}

void ImageEditor::AddArrow(ImVec2 begin, ImVec2 end)
{

}

std::shared_ptr<Texture2D> ImageEditor::Draw()
{
  for(auto& rectangle : m_rectangles)
    cv::rectangle(m_opencvImage, rectangle.topLeft, rectangle.bottomRight, cv::Scalar{0,255,255});

  for(auto& circle : m_circles)
    cv::circle(m_opencvImage, circle.center, static_cast<int>(circle.radius), cv::Scalar{0, 0, 255}); 

  for(auto& line : m_lines)
    cv::line(m_opencvImage, line.begin, line.end, cv::Scalar{0, 0, 255});

  std::shared_ptr<Texture2D> dstTexture = std::make_shared<Texture2D>(m_texture->GetTexturePtr(), "Edited texture");
  cv::directx::convertToD3D11Texture2D(m_opencvImage, dstTexture->GetTexturePtr());
  return dstTexture;
}

} // namespace medicimage
