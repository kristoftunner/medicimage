#pragma once
#include <wx/scrolwin.h>
#include <wx/frame.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/timer.h>
#include <wx/dcbuffer.h>
#include <format>
#include <chrono>
#include <vector>
#include <variant>

#include "drawing/drawing_sheet.h"
#include "renderer/texture.h"
#include "editor.h"
#include "gui/toolbox/attribute_editor.h"

using namespace medicimage;
namespace app
{

class Canvas;
class InfoDialog : public wxFrame
{
public:
  InfoDialog(Canvas* parent, const wxString& title, DrawingSheet& sheet, Editor& editor);
  void OnUpdate();
private:
  wxBoxSizer* m_sizer;
  wxStaticText* m_drawState;
  wxStaticText* m_drawCommand;
  wxStaticText* m_editorState;
  wxStaticText* m_mousePos;
  wxStaticText* m_canvasSize;
  wxStaticText* m_fps;
  DrawingSheet& m_sheet;
  Editor& m_editor;
  Canvas* m_canvas;
};

struct FPSCounter{
  float fps = 0.0f;
  std::chrono::time_point<std::chrono::steady_clock> lastTime = std::chrono::steady_clock::now();
  std::vector<float> fpsHistory;
  void Update() {
    auto now = std::chrono::steady_clock::now();
    auto diff = now - lastTime;
    lastTime = now;
    fps = 1.0f / std::chrono::duration<float>(diff).count();
    fpsHistory.push_back(fps);
    if (fpsHistory.size() > 100) {
      fpsHistory.erase(fpsHistory.begin());
    }
  }
  
  float GetAverageFPS() {
    float sum = 0.0f;
    for (auto fps : fpsHistory) {
      sum += fps;
    }
    return sum / fpsHistory.size();
  }
  
};

class Canvas : public wxWindow
{
public:
  Canvas( wxWindow *parent, wxWindowID, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);
  ~Canvas();
  
  // Event handlers coming from this window
  void OnMouseMoved(wxMouseEvent &event);
  void OnMousePressed(wxMouseEvent &event);
  void OnMouseReleased(wxMouseEvent &event);

  void OnCharInput(wxKeyEvent &event);
  void OnKeyPressed(wxKeyEvent &event);
  void OnPaint( wxPaintEvent &event );
  void OnCameraFrameUpdate(wxTimerEvent& event);

  // Event handlers coming from the parent toolbox window
  void OnScreenshot();
  void OnSave(wxCommandEvent &event);
  void OnDelete(wxCommandEvent &event);
  void OnUndo(wxCommandEvent &event);
  void OnCancel(wxCommandEvent &event);
  void OnDocumentPicked(ImageDocumentEvent& event);

  void OnDrawText(wxCommandEvent &event);
  void OnDrawIncrementalLetters(wxCommandEvent &event);
  void OnDrawArrow(wxCommandEvent &event);
  void OnDrawCircle(wxCommandEvent &event);
  void OnDrawLine(wxCommandEvent &event);
  void OnDrawMultiline(wxCommandEvent &event);
  void OnDrawRectangle(wxCommandEvent &event);
  void OnDrawSkinTemplate(wxCommandEvent &event);

  void Draw(wxDC& dc);
  CameraAPI* GetCamera() { return m_editor.GetCamera(); }
  std::mutex& GetCameraMutex() { return m_editor.GetCameraMutex(); }
  wxPoint GetMousePoint() { return m_mousePoint; }
  wxSize GetCanvasSize() { return m_canvasSize; }
  float GetFPS() { return m_fpsCounter.GetAverageFPS(); }
private:
  std::unique_ptr<Image2D> DummyDraw();
  void UpdateAttributeEditor();
  glm::vec2 CalcCorrectedMousePos(glm::vec2 pos);
  wxSize m_canvasSize;
  float m_canvasScale = 1.0f;
  glm::vec2 m_canvasBorder{ 0.0f, 0.0f };
  bool m_mouseDown = false;
  InfoDialog* m_dialog;
  Editor m_editor;
  wxTimer m_frameUpdateTimer;
  wxPoint m_mousePoint;
  FPSCounter m_fpsCounter;

  wxPoint m_firstPoint, m_secondPoint;
};

class EditorPanel : public wxPanel
{
public:
  EditorPanel(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
  ~EditorPanel() = default;

  CameraAPI* GetCamera() { return m_canvas->GetCamera(); }
  std::mutex& GetCameraMutex() { return m_canvas->GetCameraMutex(); }
  Canvas* GetCanvas() { return m_canvas; }
private:
  Canvas* m_canvas;
};
};
