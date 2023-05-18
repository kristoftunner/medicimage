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
private:
  std::unique_ptr<Image2D> m_image; 
  medicimage::DrawingSheet m_drawingSheet;  
  bool m_mouseDown = false;
};

}
