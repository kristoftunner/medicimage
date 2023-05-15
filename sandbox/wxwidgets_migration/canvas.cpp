#include "canvas.h"
#include <image_handling/image_editor.h>
#include <renderer/texture.h>

#include <wx/wxprec.h>
#include <wx/dcclient.h>
#include <wx/wx.h>
#include <wx/image.h>
#include <wx/dcmemory.h>

using namespace medicimage;
namespace app
{
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
  auto image = std::make_unique <Image2D>("Checkerboard.png");
  wxBitmap::Rescale(image->GetBitmap(), {600, 500});
  m_drawingSheet.SetDrawingSheetSize({image->GetWidth(), image->GetHeight()});
  m_drawingSheet.SetDocument(std::make_unique<ImageDocument>(std::move(image)), {600, 500});
}

Canvas::~Canvas()
{
}

void Canvas::OnMouseEvent(wxMouseEvent &event)
{
  wxClientDC dc(this);
  PrepareDC(dc);

  auto image = m_drawingSheet.Draw();

  const wxPoint pt(event.GetLogicalPosition(dc));
  if(event.LeftDown())
  {
    m_drawingSheet.OnMouseButtonPressed({pt.x, pt.y});
  }
  else if(event.LeftIsDown())
  {
    m_drawingSheet.OnMouseButtonDown({pt.x, pt.y});
  }
  else if(event.LeftUp())
  {
    m_drawingSheet.OnMouseButtonReleased({pt.x, pt.y});
  }
  m_drawingSheet.OnUpdate();

  dc.DrawBitmap(image->GetBitmap(), 0, 0); 
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
    ;
  }
  else
  {
    wxMessageBox("Image not found");
  }

  m_drawingSheet.SetDrawCommand(DrawCommand::DRAW_LINE); 
  //dc.DrawRectangle(100, 100, 100, 100);
  //dc.DrawText("Hello World", 0, 0);
}

}
