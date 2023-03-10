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

struct Text
{
  std::string text;
  cv::Point bottomLeft;
  int fontSize;
  Color color;
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
  void AddText(const std::string& text, ImVec2 bottomLeft, PrimitiveAddingType addType,const int fontSize, const Color color={0,0,0}); // TODO: change const std::string& to std::string_view 
  void ClearDrawing();
  static std::shared_ptr<Texture2D> AddImageFooter(const std::string& footerText, std::shared_ptr<Texture2D> texture);
  std::shared_ptr<Texture2D> Draw();
private:
  static cv::UMat AddFooter(cv::UMat image, const std::string& footerText);
  std::unique_ptr<Texture2D> m_texture;
  cv::UMat m_opencvImage;
  static constexpr int m_sideBorder = 20;
  static constexpr int m_topBorder = 20;
  static constexpr int m_bottomBorder = 100;
  // primitives to draw on the image in the Draw call
  std::vector<Line> m_lines;
  std::vector<Circle> m_circles;
  std::vector<Rectangle> m_rectangles;
  std::vector<Line> m_arrows;
  std::vector<Text> m_texts;
  
  std::optional<Line> m_tempLine;
  std::optional<Circle> m_tempCircle;
  std::optional<Rectangle> m_tempRectangle;
  std::optional<Line> m_tempArrow;
  std::optional<Text> m_tempText;
};

} // namespace medicimage
