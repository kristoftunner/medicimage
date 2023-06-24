#pragma once
#include <glm/glm.hpp>
#include <string>
#include <filesystem>
#include <mutex>
#include <thread>
#include <optional>
#include <atomic>

#include "input/key_codes.h"
#include "renderer/texture.h"
#include "drawing/drawing_sheet.h"
#include "image_handling/image_saver.h"
#include "core/utils.h"
#include "camera/opencv_camera.h"
#include "gui/custom_events.h"

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
  ~Editor()
  {
    m_closeRequested = true;
    m_cameraUpdateThread.join();
  }
  void Init();
  void UpdateAppFolder(const std::filesystem::path& appFolder);

  void OnMouseMoved(const glm::vec2& pos);
  void OnMousePressed(const glm::vec2& pos);
  void OnMouseReleased(const glm::vec2& pos);

  void OnCharInput(const std::string& character);
  void OnKeyPressed(KeyCode key);

  // Event handlers coming from the parent toolbox window
  std::optional<ImageDocumentEvent> OnScreenshot();
  void OnScreenshotDone();
  std::optional<ImageDocumentEvent> OnSave();
  
  std::optional<ImageDocumentEvent> OnDelete();
  bool CanDelete();
  void OnUndo();
  void OnCancel();
  bool CanUndo();
  void OnDocumentPicked(const ImageDocument& document);

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

  CameraAPI* GetCamera() { return &m_camera; }
  std::mutex& GetCameraMutex() { return m_cameraMutex; }
  std::vector<Entity> GetSelectedEntities() {return m_drawingSheet.GetSelectedEntities();}

  std::unique_ptr<Image2D> Draw();
  DrawingSheet& GetDrawingSheet() { return m_drawingSheet; } // Getter for the debug dialog box
  std::string GetStateName() const;
private:
  glm::vec2 ClampMousePosition(const glm::vec2& pos);
  bool m_mouseDown = false;
  EditorState m_state = EditorState::SHOW_CAMERA; // default state is showing the camrea
  DrawingSheet m_drawingSheet;
  bool m_newDrawingAvailable = false;
  ImageDocument m_activeDocument;
  
  OpenCvCamera m_camera;
  std::mutex m_cameraMutex;
  std::unique_ptr<Image2D> m_cameraFrame;
  std::mutex m_cameraFrameMutex;
  std::atomic<float> m_cameraFrameRate = 0.0f;
  std::atomic<bool> m_newFrameAvailable = false;
  std::atomic<bool> m_closeRequested = false;
  std::thread m_cameraUpdateThread;
};

} // namespace app
