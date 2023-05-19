#include "canvas.h"
#include <image_handling/image_editor.h>
#include <renderer/texture.h>
#include "toolbox/toolbox.h"

#include <wx/wxprec.h>
#include <wx/dcclient.h>
#include <wx/dcbuffer.h>
#include <wx/wx.h>
#include <wx/image.h>
#include <wx/dcmemory.h>

using namespace medicimage;
namespace app
{
Canvas::Canvas( wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size )
  : wxScrolledWindow( parent, id, pos, size )
{
  SetBackgroundStyle(wxBG_STYLE_PAINT);
  SetBackgroundColour( *wxWHITE );
  SetCursor(wxCursor(wxCURSOR_ARROW));
  auto image = std::make_unique <Image2D>("Checkerboard.png");
  wxBitmap::Rescale(image->GetBitmap(), {600, 500});
  m_drawingSheet.SetDrawingSheetSize({image->GetWidth(), image->GetHeight()});
  m_drawingSheet.SetDocument(std::make_unique<ImageDocument>(std::move(image)), {600, 500});
  m_drawingSheet.SetDrawCommand(DrawCommand::DRAW_LINE); 

  Bind(wxEVT_PAINT, &Canvas::OnPaint, this);
  Bind(wxEVT_LEFT_DOWN, &Canvas::OnMousePressed, this);
  Bind(wxEVT_LEFT_UP, &Canvas::OnMouseReleased, this);
  Bind(wxEVT_MOTION, &Canvas::OnMouseMoved, this);
  Bind(TOOLBOX_SCREENSHOT, &Canvas::OnScreenshot, this);
}

Canvas::~Canvas()
{
}

void Canvas::OnMouseMoved(wxMouseEvent &event)
{
  if(m_mouseDown)
  {
    wxClientDC dc(this);
    PrepareDC(dc);

    const wxPoint pt(event.GetLogicalPosition(dc));
    m_drawingSheet.OnMouseButtonDown({pt.x, pt.y});
    m_drawingSheet.OnUpdate();
    Refresh();
  }
}

void Canvas::OnMousePressed(wxMouseEvent &event)
{
  wxClientDC dc(this);
  PrepareDC(dc);

  const wxPoint pt(event.GetLogicalPosition(dc));
  m_drawingSheet.OnMouseButtonPressed({pt.x, pt.y});

  m_mouseDown = true;
  Refresh();
}

void Canvas::OnMouseReleased(wxMouseEvent &event)
{
  if(m_mouseDown)
  {
    wxClientDC dc(this);
    PrepareDC(dc);

    const wxPoint pt(event.GetLogicalPosition(dc));
    m_drawingSheet.OnMouseButtonReleased({pt.x, pt.y});
  
    m_mouseDown = false;
    Refresh();
  }
}

void Canvas::OnScreenshot(wxCommandEvent &event)
{
  wxLogDebug("Screenshot event");
}

void Canvas::OnPaint(wxPaintEvent &event)
{
  wxLogDebug("Paint event");
  wxAutoBufferedPaintDC dc(this);
  PrepareDC(dc);
  dc.SetBackground(*wxWHITE_BRUSH);
  dc.Clear();
  dc.SetPen(*wxBLACK_PEN);
  dc.SetBrush(*wxTRANSPARENT_BRUSH);

  auto image = m_drawingSheet.Draw();
  dc.DrawBitmap(image->GetBitmap(), 0, 0);
  
}

}
