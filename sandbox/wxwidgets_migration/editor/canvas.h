#pragma once
#include <wx/scrolwin.h>
#include <wx/frame.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/timer.h>
#include <format>

#include "drawing/drawing_sheet.h"
#include "renderer/texture.h"
#include "editor.h"
#include "toolbox/attribute_editor.h"

using namespace medicimage;
namespace app
{
class InfoDialog : public wxFrame
{
public:
  InfoDialog(wxWindow* parent, const wxString& title, DrawingSheet& sheet, Editor& editor);
  void OnUpdate();
private:
  wxBoxSizer* m_sizer;
  wxStaticText* m_drawState;
  wxStaticText* m_drawCommand;
  wxStaticText* m_editorState;
  DrawingSheet& m_sheet;
  Editor& m_editor;
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
  void OnScreenshot(wxCommandEvent &event);
  void OnSave(wxCommandEvent &event);
  void OnDelete(wxCommandEvent &event);
  void OnUndo(wxCommandEvent &event);
  void OnDocumentPicked(ImageDocumentEvent& event);

  void OnDrawText(wxCommandEvent &event);
  void OnDrawIncrementalLetters(wxCommandEvent &event);
  void OnDrawArrow(wxCommandEvent &event);
  void OnDrawCircle(wxCommandEvent &event);
  void OnDrawLine(wxCommandEvent &event);
  void OnDrawMultiline(wxCommandEvent &event);
  void OnDrawRectangle(wxCommandEvent &event);
  void OnDrawSkinTemplate(wxCommandEvent &event);

  CameraAPI* GetCamera() { return m_editor.GetCamera(); }
  std::mutex& GetCameraMutex() { return m_editor.GetCameraMutex(); }
private:
  void UpdateAttributeEditor();
  bool m_mouseDown = false;
  InfoDialog* m_dialog;
  Editor m_editor;
  wxTimer m_frameUpdateTimer;
};

class EditorPanel : public wxPanel
{
public:
  EditorPanel(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
  ~EditorPanel() = default;

  CameraAPI* GetCamera() { return m_canvas->GetCamera(); }
  std::mutex& GetCameraMutex() { return m_canvas->GetCameraMutex(); }
private:
  Canvas* m_canvas;
};
};
