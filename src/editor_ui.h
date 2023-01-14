#pragma once

#include "layer.h"
#include "imgui.h"
#include "texture.h"
#include "image_editor.h"

#include <memory>
#include <vector>

namespace medicimage
{

class EditorUI : public Layer
{
public:
  EditorUI();
  ~EditorUI();
  void OnUpdate() override;
  void OnAttach() override;
  void OnDetach() override;
  void OnImguiRender() override;
private:
  std::string GenerateImageName();
  std::unique_ptr<Texture2D> m_circleIcon, m_editImageIcon, m_lineIcon, m_pencilIcon, m_screenshotIcon,
    m_rectangleIcon, m_arrowIcon, m_addTextIcon;

  std::unique_ptr<Texture2D> m_currentFrame;
  std::string m_uuid = "123456"; 
  int m_capturedImageIndex = 0;
  std::vector<std::shared_ptr<Texture2D>> m_capturedImages;
  ImageEditor m_imageEditor;  
  // internal state variables for the drawing functions 
  bool m_drawLineRequested = false;
  bool m_drawCircleRequested = false;
  bool m_drawRectangleRequested = false;
  bool m_addTextRequested = false;

  // for drawing a circle/rectangle/line/arrow we need only 2 points
  // for adding a text the topleft corner is enough 
  ImVector<ImVec2> m_cursorEditPoints;
};
  
} // namespace medicimage
