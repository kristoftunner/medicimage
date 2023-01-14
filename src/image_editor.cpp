#include "image_editor.h"
#include "opencv2/core/directx.hpp"
#include "opencv2/core/ocl.hpp"
#include <opencv2/imgproc.hpp>
#include "log.h"
#include <fstream>

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

void ImageEditor::SetTextureForEditing(Texture2D* texture)
{
  m_texture = texture;
}

void ImageEditor::Init(ID3D11Device* device)
{
  if(cv::ocl::haveOpenCL())
    cv::directx::ocl::initializeContextFromD3D11Device(device);
  else
    APP_CORE_ERR("Do not have OpenCL device!!");
}

void ImageEditor::DrawRectangle(ImVec2 topLeft, ImVec2 bottomRight)
{
  auto checkInput = [&](float input) { return ((input <= 1) && (input >= 0)); };
  if(!checkInput(topLeft.x) || !checkInput(topLeft.y) || !checkInput(bottomRight.x) || !checkInput(bottomRight.y))
    return;

  D3D11_TEXTURE2D_DESC inputDesc, outputDesc;
  m_texture->GetTexturePtr()->GetDesc(&inputDesc);
  cv::UMat textureOcv;
  cv::directx::convertFromD3D11Texture2D(m_texture->GetTexturePtr(), textureOcv);
  int textureWidth = m_texture->GetWidth();
  int textureHeight = m_texture->GetHeight();
  cv::Point rectangleTopLeft = {static_cast<int>(topLeft.x * textureWidth), static_cast<int>(topLeft.y * textureHeight)}; 
  cv::Point rectangleBottomRight = {static_cast<int>(bottomRight.x * textureWidth), static_cast<int>(bottomRight.y * textureHeight)}; 
  cv::rectangle(textureOcv, rectangleTopLeft, rectangleBottomRight, cv::Scalar{0,255,255});
  
  //cv::String strMode = cv::format("hello");
  //cv::putText(textureOcv, strMode, cv::Point(0, 20), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 200), 2);
  cv::directx::convertToD3D11Texture2D(textureOcv, m_texture->GetTexturePtr());
  m_texture->GetTexturePtr()->GetDesc(&outputDesc);
  DumpTexture(m_texture->GetTexturePtr());
}

void ImageEditor::DrawCircle(ImVec2 orig, float radius)
{

}

void ImageEditor::DrawLine(ImVec2 begin, ImVec2 end)
{

}
void ImageEditor::DrawArrow(ImVec2 begin, ImVec2 end)
{

}
  
} // namespace medicimage
