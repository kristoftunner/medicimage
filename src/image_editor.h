#pragma once

#include "texture.h"
#include "imgui.h"
#include "image_editor.h"
#include <d3d11.h>
#include "opencv2/core/directx.hpp"
#include "opencv2/core/ocl.hpp"
#include <opencv2/imgproc.hpp>

#include <memory>
#include <vector>
#include <optional>

namespace medicimage
{

struct Color
{
  int red, blue, green;
};

struct PrimitiveAttributes
{
  int thickness = 1; // in pixels
  Color color = {0,0,0};
};

struct Line
{
  cv::Point begin;
  cv::Point end;
  PrimitiveAttributes attributes;
};

struct Circle
{
  cv::Point center;
  float radius;
  PrimitiveAttributes attributes;
};

struct Rectangle
{
  cv::Point topLeft;
  cv::Point bottomRight;
  PrimitiveAttributes attributes;
};

enum class PrimitiveAddingType {TEMPORARY, PERMANENT};

class ImageEditor
{
public:
  ImageEditor() = default;
  void Init(ID3D11Device* device);
  void SetTextureForEditing(std::unique_ptr<Texture2D> texture);
  // topLeft, width and height are relative to the texture size, between 0-1
  void AddRectangle(ImVec2 topLeft, ImVec2 bottomRight, PrimitiveAddingType addType, const int thickness = 1, const Color color = {0,0,0});
  void AddCircle(ImVec2 center, float radius, PrimitiveAddingType addType, const int thickness = 1, const Color color= {0,0,0}); // orig is sized between 0-1
  void AddLine(ImVec2 begin, ImVec2 end, PrimitiveAddingType addType, const int thickness = 1, const Color color= {0,0,0});
  void AddArrow(ImVec2 begin, ImVec2 end, PrimitiveAddingType addType, const int thickness = 1, const Color color= {0,0,0});
  static std::shared_ptr<Texture2D> AddImageFooter(const std::string& watermark, std::shared_ptr<Texture2D> texture);
  std::shared_ptr<Texture2D> Draw();
private:
  std::unique_ptr<Texture2D> m_texture;
  cv::UMat m_opencvImage;

  // primitives to draw on the image in the Draw call
  std::vector<Line> m_lines;
  std::vector<Circle> m_circles;
  std::vector<Rectangle> m_rectangles;
  std::vector<Line> m_arrows;

  std::optional<Line> m_tempLine;
  std::optional<Circle> m_tempCircle;
  std::optional<Rectangle> m_tempRectangle;
  std::optional<Line> m_tempArrow;
};

} // namespace medicimage
