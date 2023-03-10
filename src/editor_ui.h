#pragma once

#include "layer.h"
#include "imgui.h"
#include "texture.h"
#include "image_editor.h"
#include "opencv_camera.h"
#include "image_saver.h"
#include "log.h"
#include "utils.h"
#include "key_event.h"

#include <array>
#include <memory>
#include <vector>

namespace medicimage
{

enum class EditorState{SHOW_CAMERA, EDITING, SCREENSHOT};
enum class EditingCommandType{INITIAL, DRAWING};
enum class DrawCommandType{DO_NOTHING, DRAW_LINE, DRAW_CIRCLE, DRAW_RECTANGLE, DRAW_ARROW, ADD_TEXT, DELETE_IMAGE, UNDO};
enum class DrawCommandState{INITIAL, FIRST_CLICK, MOUSE_DOWN, SECOND_CLICK, FINISH};
struct DrawCommand 
{
  DrawCommandType commandType;
  DrawCommandState commandState; 
};

class EditorUI : public Layer
{
public:
  EditorUI();
  ~EditorUI();
  void OnUpdate() override;
  void OnAttach() override;
  void OnDetach() override;
  void OnEvent(Event* event) override;
  void OnImguiRender() override;
private:
  void Draw(PrimitiveAddingType addType, ImVec2 imageSize);
  bool OnKeyTextInputEvent(KeyTextInputEvent* e);
  bool OnKeyPressedEvent(KeyPressedEvent* e);
  void ShowImageWindow();
  void ShowToolbox();
  void ShowThumbnails();
  struct CallbackFunctions // for ImGui textinput callback 
  {
    static int EnterPressedCallback(ImGuiInputTextCallbackData* data)
    {
      if (data->EventFlag == ImGuiInputTextFlags_CallbackAlways)
      {
        if (data->EventKey == ImGuiKey_2)
          APP_CORE_TRACE("Key 2 is clicked");
        if (data->EventKey == ImGuiKey_Enter)
          s_enterPressed = true;
      }
      return 0;
    }
  };

private:

  static bool s_enterPressed;

  std::unique_ptr<Texture2D> m_circleIcon, m_lineIcon, m_pencilIcon, m_saveIcon, m_deleteIcon,
    m_rectangleIcon, m_arrowIcon, m_addTextIcon, m_screenshotIcon, m_undoIcon;

  std::unique_ptr<Texture2D> m_activeOriginalImage;
  std::shared_ptr<Texture2D> m_activeEditedImage;
  OpenCvCamera m_camera = OpenCvCamera(0);
   
  // UI editor state specific members
  ImageEditor m_imageEditor; 
  
  EditorState m_editorState = EditorState::SHOW_CAMERA;
  DrawCommand m_activeCommand;
  Timer m_timer;
  std::array<char,128> m_inputText; 
  ImFont* m_largeFont;
  ImFont* m_smallFont;
  ImVec4 m_defaultFrameBgColor;
  ImVec4 m_toolUsedBgColor = {5.0/255.0,247.0/255.0,3.0/255.0,138.0/255.0};

  // drawing specific members
  int m_thickness = 3;
  Color m_color = {255,0,0};
  std::string m_editText = "";
  int m_drawnTextFontSize = 4;  
  // for drawing a circle/rectangle/line/arrow we need only 2 points
  // for adding a text the topleft corner is enough 
  ImVector<ImVec2> m_cursorEditPoints;

  std::unique_ptr<ImageSaverContainer> m_imageSavers;
  AppConfig m_appConfig;
};
  
} // namespace medicimage
