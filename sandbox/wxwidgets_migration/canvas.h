#pragma once

#include "drawing/drawing_sheet.h"
#include "renderer/texture.h"

#include <wx/scrolwin.h>

using namespace medicimage;
namespace app
{
class Canvas : public wxScrolledWindow
{
public:
  Canvas( wxWindow *parent, wxWindowID, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);
  ~Canvas();

  void OnMouseMoved(wxMouseEvent &event);
  void OnMousePressed(wxMouseEvent &event);
  void OnMouseReleased(wxMouseEvent &event);

  void OnPaint( wxPaintEvent &event );

  void OnScreenshot(wxCommandEvent &event);
  void OnSave(wxCommandEvent &event);
  void OnDelete(wxCommandEvent &event);
  void OnUndo(wxCommandEvent &event);

  void OnDrawText(wxCommandEvent &event);
  void OnDrawIncrementalLetters(wxCommandEvent &event);
  void OnDrawArrow(wxCommandEvent &event);
  void OnDrawCircle(wxCommandEvent &event);
  void OnDrawLine(wxCommandEvent &event);
  void OnDrawMultiline(wxCommandEvent &event);
  void OnDrawRectangle(wxCommandEvent &event);
  void OnDrawSkinTemplate(wxCommandEvent &event);

private:
  std::unique_ptr<Image2D> m_image; 
  medicimage::DrawingSheet m_drawingSheet;  
  bool m_mouseDown = false;
};

}
