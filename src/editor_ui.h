#pragma once

#include "layer.h"
#include "imgui.h"
#include "texture.h"
#include "image_editor.h"
#include "opencv_camera.h"
#include "image_saver.h"

#include <memory>
#include <vector>

namespace medicimage
{

enum class EditCommandType{DO_NOTHING, DRAW_LINE, DRAW_CIRCLE, DRAW_RECTANGLE, ADD_TEXT, START_EDITING_IMAGE, SAVE_IMAGE};
enum class EditState{START_EDIT, FIRST_CLICK, MOUSE_DOWN, DONE};
struct EditCommand
{
  EditCommandType editType;
  EditState editState; // this is used for the drawing command for indicating that the second mouse click is done or not
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
  void Draw(PrimitiveAddingType addType, ImVec2 imageSize);
  std::unique_ptr<Texture2D> m_circleIcon, m_startEditingIcon, m_lineIcon, m_pencilIcon, m_saveIcon,
    m_rectangleIcon, m_arrowIcon, m_addTextIcon;

  std::unique_ptr<Texture2D> m_currentFrame;
  std::shared_ptr<Texture2D> m_currentEditedFrame;
  std::vector<std::unique_ptr<Texture2D>> m_capturedImages;
  OpenCvCamera m_camera = OpenCvCamera(0);
  
  std::string m_uuid = "123456"; 
  int m_capturedImageIndex = 0;
  
  ImageEditor m_imageEditor; 
  EditCommand m_currentCommand = {EditCommandType::DO_NOTHING, EditState::DONE};
  bool m_inEditMode = false;

  // internal state variables for the drawing functions -> TODO: do some dispatching logic 
  bool m_drawLineRequested = false;
  bool m_drawCircleRequested = false;
  bool m_drawRectangleRequested = false;
  bool m_addTextRequested = false;

  int m_thickness = 1;
  Color m_color = {0,0,0};
  // for drawing a circle/rectangle/line/arrow we need only 2 points
  // for adding a text the topleft corner is enough 
  ImVector<ImVec2> m_cursorEditPoints;

  ImageSaverContainer m_imageSavers;
};
  
} // namespace medicimage
