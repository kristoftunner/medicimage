#pragma once

#include "texture.h"
#include <memory>
#include <vector>
#include "imgui.h"
#include "image_editor.h"
#include <d3d11.h>
#include "opencv2/core/directx.hpp"
#include "opencv2/core/ocl.hpp"
#include <opencv2/imgproc.hpp>


namespace medicimage
{
struct Line
{
  cv::Point begin;
  cv::Point end;
};

struct Circle
{
  cv::Point center;
  float radius;
};

struct Rectangle
{
  cv::Point topLeft;
  cv::Point bottomRight;
};

class ImageEditor
{
public:
  ImageEditor() = default;
  void Init(ID3D11Device* device);
  void SetTextureForEditing(std::unique_ptr<Texture2D> texture);
  // topLeft, width and height are relative to the texture size, between 0-1
  void AddRectangle(ImVec2 topLeft, ImVec2 bottomRight);
  void AddCircle(ImVec2 center, float radius); // orig is sized between 0-1
  void AddLine(ImVec2 begin, ImVec2 end);
  void AddArrow(ImVec2 begin, ImVec2 end);
  std::shared_ptr<Texture2D> Draw();
private:
  std::unique_ptr<Texture2D> m_texture;
  cv::UMat m_opencvImage;

  // primitives to draw on the image in the Draw call
  std::vector<Line> m_lines;
  std::vector<Circle> m_circles;
  std::vector<Rectangle> m_rectangles;
  std::vector<Line> m_arrows;
};

} // namespace medicimage
