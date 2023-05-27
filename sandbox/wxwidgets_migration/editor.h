#pragma once
#include "input/key_codes.h"
#include "renderer/texture.h"
#include "drawing/drawing_sheet.h"
#include "image_handling/image_saver.h"
#include "core/utils.h"
#include "camera/opencv_camera.h"

#include <glm/glm.hpp>
#include <string>
#include <filesystem>
#include <mutex>
#include <thread>
#include <atomic>

namespace app
{

using namespace medicimage;

enum class EditorState
{
  SHOW_CAMERA, 
  IMAGE_SELECTION, 
  EDITING, 
  SCREENSHOT
};

class Editor
{
public:
  Editor() = default;
  ~Editor() = default;
  void Init();
  void UpdateAppFolder(const std::filesystem::path& appFolder);

  void OnMouseMoved(const glm::vec2& pos);
  void OnMousePressed(const glm::vec2& pos);
  void OnMouseReleased(const glm::vec2& pos);

  void OnCharInput(const std::string& character);
  void OnKeyPressed(KeyCode key);

  // Event handlers coming from the parent toolbox window
  void OnScreenshot();
  void OnScreenshotDone();
  void OnSave();
  // delete is for deleting image during IMAGE_SELECTION
  void OnDelete();
  // undo is undoing changes during EDITING and revert IMAGE_SELECTION
  void OnUndo();
  void OnImageSelected(int index);

  void OnDrawText();
  void OnDrawIncrementalLetters();
  void OnDrawArrow();
  void OnDrawCircle();
  void OnDrawLine();
  void OnDrawMultiline();
  void OnDrawRectangle();
  void OnDrawSkinTemplate();
  void OnDrawButtonPushed(DrawCommand command);

  bool IsCameraFrameUpdated() const { return m_newFrameAvailable; }
  void UpdatedFrame() { 
    m_newFrameAvailable = false; 
  }
  bool IsDrawingUpdated() const { return m_newDrawingAvailable; }
  void UpdatedDrawing() { 
    m_newDrawingAvailable = false; 
  }

  std::unique_ptr<Image2D> Draw();
  DrawingSheet& GetDrawingSheet() { return m_drawingSheet; } // Getter for the debug dialog box
  std::string GetStateName() const;
private:
  bool m_mouseDown = false;
  EditorState m_state = EditorState::SHOW_CAMERA; // default state is showing the camrea
  DrawingSheet m_drawingSheet;
  std::unique_ptr<ImageSaverContainer> m_imageSavers;
  AppConfig m_appConfig;
  bool m_newDrawingAvailable = false;
  
  OpenCvCamera m_camera;
  std::unique_ptr<Image2D> m_cameraFrame;
  std::mutex m_cameraFrameMutex;
  std::atomic<float> m_cameraFrameRate = 0.0f;
  std::atomic<bool> m_newFrameAvailable = false;
  std::thread m_cameraUpdateThread;
};

} // namespace app
