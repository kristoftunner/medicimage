#pragma once

#include "layer.h"
#include "imgui.h"
#include "texture.h"
#include "image_editor.h"

#include <memory>
#include <vector>

namespace medicimage
{

enum class EditCommandType{DO_NOTHING, DRAW_LINE, DRAW_CIRCLE, DRAW_RECTANGLE, ADD_TEXT, START_EDITING_IMAGE, SAVE_IMAGE};

struct EditCommand
{
  EditCommandType editType;
  bool editing; // this is used for the drawing command for indicating that the second mouse click is done or not
};

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
  void DispatchDrawingCommand();
  std::unique_ptr<Texture2D> m_circleIcon, m_startEditingIcon, m_lineIcon, m_pencilIcon, m_saveIcon,
    m_rectangleIcon, m_arrowIcon, m_addTextIcon;

  std::unique_ptr<Texture2D> m_currentFrame;
  std::shared_ptr<Texture2D> m_currentEditedFrame;
  std::vector<std::unique_ptr<Texture2D>> m_capturedImages;
  
  std::string m_uuid = "123456"; 
  int m_capturedImageIndex = 0;
  
  ImageEditor m_imageEditor; 
  EditCommand m_currentCommand = {EditCommandType::DO_NOTHING, false};
  bool m_inEditMode = false;

  // internal state variables for the drawing functions -> TODO: do some dispatching logic 
  bool m_drawLineRequested = false;
  bool m_drawCircleRequested = false;
  bool m_drawRectangleRequested = false;
  bool m_addTextRequested = false;

  // for drawing a circle/rectangle/line/arrow we need only 2 points
  // for adding a text the topleft corner is enough 
  ImVector<ImVec2> m_cursorEditPoints;
};
  
} // namespace medicimage
