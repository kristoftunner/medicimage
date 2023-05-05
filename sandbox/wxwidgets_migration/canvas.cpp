#include "canvas.h"
#include <wx/wxprec.h>
#include <wx/dcclient.h>
#include <wx/wx.h>
#include <wx/image.h>
#include <wx/dcmemory.h>

wxBEGIN_EVENT_TABLE(Canvas, wxScrolledWindow)
  EVT_PAINT(Canvas::OnPaint)
  EVT_MOUSE_EVENTS(Canvas::OnMouseEvent)
wxEND_EVENT_TABLE()

Canvas::Canvas( wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size )
  : wxScrolledWindow( parent, id, pos, size )
{
  SetBackgroundColour( *wxWHITE );
  SetCursor(wxCursor(wxCURSOR_ARROW));
  wxInitAllImageHandlers();  
}

Canvas::~Canvas()
{
}

void Canvas::OnMouseEvent(wxMouseEvent &event)
{
  wxClientDC dc(this);
  PrepareDC(dc);
 
  const wxPoint pt(event.GetLogicalPosition(dc));
  if(event.ButtonDown(wxMOUSE_BTN_LEFT))
  {
    wxLogDebug("Left button down at (%d, %d)", pt.x, pt.y);
    dc.SetBackground(*wxWHITE_BRUSH);
    dc.Clear();
    dc.SetPen(*wxRED_PEN);
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(200, 200, 100, 100);
    dc.DrawText("Hello World", 0, 0);

  }
  else if(event.ButtonDown(wxMOUSE_BTN_RIGHT))
    wxLogDebug("Right button down at (%d, %d)", pt.x, pt.y);
  
}

void Canvas::OnPaint(wxPaintEvent &event)
{
  wxPaintDC dc(this);
  PrepareDC(dc);
  dc.SetBackground(*wxWHITE_BRUSH);
  dc.Clear();
  dc.SetPen(*wxBLACK_PEN);
  dc.SetBrush(*wxTRANSPARENT_BRUSH);
  
  wxImage image("Checkerboard.png");
  if(image.IsOk())
  {
    wxBitmap bitmap(image);
    wxBitmap::Rescale(bitmap, {1920, 1080});

    wxMemoryDC memDC(bitmap);
    memDC.SetBrush(dc.GetBrush());
    memDC.SetPen(dc.GetPen());
    memDC.DrawRectangle(200, 200, 100, 100);
    memDC.SelectObject(wxNullBitmap);
    ////wxColour c;
    //if (memDC.GetPixel(dc.FromDIP(11), dc.FromDIP(11), &c))
    //{
    //  memDC.SetBrush(wxColour(128, 128, 0));
    //  memDC.FloodFill(dc.FromDIP(11), dc.FromDIP(11), c, wxFLOOD_SURFACE);
    //}


    dc.DrawBitmap(bitmap, 0, 0);
  }
  else
  {
    wxMessageBox("Image not found");
  }
  
  dc.DrawRectangle(100, 100, 100, 100);
  dc.DrawText("Hello World", 0, 0);
}
