#pragma once

#include "drawing/drawing_sheet.h"
#include "renderer/texture.h"

#include <wx/scrolwin.h>

using namespace medicimage;
class Canvas : public wxScrolledWindow
{
public:
  Canvas( wxWindow *parent, wxWindowID, const wxPoint &pos, const wxSize &size );
  ~Canvas();

  void OnMouseEvent(wxMouseEvent &event);
  void OnPaint( wxPaintEvent &event );
private:
  std::unique_ptr<Image2D> m_image; 
  medicimage::DrawingSheet m_drawingSheet;  
  wxDECLARE_EVENT_TABLE();
};

