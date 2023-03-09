#pragma once

#include "texture.h"
#include "imgui.h"

#include <d3d11.h>
#include "opencv2/core/directx.hpp"
#include "opencv2/core/ocl.hpp"
#include <opencv2/imgproc.hpp>

#include <memory>
#include <vector>
#include <optional>
#include <glm/glm.hpp>

namespace medicimage
{

class ImageEditor
{
public:
  ImageEditor() = default;
  void Init(ID3D11Device* device);
  // topLeft, width and height are relative to the texture size, between 0-1
  static std::unique_ptr<Texture2D> AddImageFooter(const std::string& footerText, Texture2D* texture);
  static std::unique_ptr<Texture2D> ReplaceImageFooter(const std::string& footerText, Texture2D* texture);
  static std::unique_ptr<Texture2D> RemoveFooter(Texture2D* texture);

  static void DrawCircle(Texture2D* texture, glm::vec2 center, float radius, glm::vec4 color, float thickness, bool filled);
  static void DrawRectangle(Texture2D* texture, glm::vec2 topleft, glm::vec2 bottomright, glm::vec4 color, float thickness, bool filled);
  static void DrawArrow(Texture2D* texture, glm::vec2 begin, glm::vec2 end, glm::vec4 color, float thickness, double tipLengith);
private:
  static cv::UMat AddFooter(cv::UMat image, const std::string& footerText);
  static constexpr int s_sideBorder = 20;
  static constexpr int s_topBorder = 20;
  static constexpr int s_bottomBorder = 100;

  // TODO: move this into a better place
  cv::ocl::Context m_context;
};

} // namespace medicimage
