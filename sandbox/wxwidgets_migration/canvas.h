#pragma once

#include <wx/scrolwin.h>

class Canvas : public wxScrolledWindow
{
public:
  Canvas( wxWindow *parent, wxWindowID, const wxPoint &pos, const wxSize &size );
  ~Canvas();

  void OnMouseEvent(wxMouseEvent &event);
  void OnPaint( wxPaintEvent &event );
private:
  wxDECLARE_EVENT_TABLE();
};

