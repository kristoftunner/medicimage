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
  Bind(TOOLBOX_SAVE, &Canvas::OnSave, this);
  Bind(TOOLBOX_DELETE, &Canvas::OnDelete, this);
  Bind(TOOLBOX_UNDO, &Canvas::OnUndo, this);

  Bind(TOOLBOX_DRAW_TEXT, &Canvas::OnDrawText, this);
  Bind(TOOLBOX_DRAW_LETTERS, &Canvas::OnDrawIncrementalLetters, this);
  Bind(TOOLBOX_DRAW_ARROW, &Canvas::OnDrawArrow, this);
  Bind(TOOLBOX_DRAW_CIRCLE, &Canvas::OnDrawCircle, this);
  Bind(TOOLBOX_DRAW_LINE, &Canvas::OnDrawLine, this);
  Bind(TOOLBOX_DRAW_MULTILINE, &Canvas::OnDrawMultiline, this);
  Bind(TOOLBOX_DRAW_RECTANGLE, &Canvas::OnDrawRectangle, this);
  Bind(TOOLBOX_DRAW_SKIN_TEMPLATE, &Canvas::OnDrawSkinTemplate, this);
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

void Canvas::OnScreenshot(wxCommandEvent &event)
{
  wxLogDebug("Canvas: OnScreenshot");
}

void Canvas::OnSave(wxCommandEvent &event)
{ 
  wxLogDebug("Canvas: OnSave");
}

void Canvas::OnDelete(wxCommandEvent &event)
{
  wxLogDebug("Canvas: OnDelete");
}

void Canvas::OnUndo(wxCommandEvent &event)
{
  wxLogDebug("Canvas: OnUndo");
}

void Canvas::OnDrawText(wxCommandEvent &event)
{
  wxLogDebug("Canvas: OnDrawText");
}
void Canvas::OnDrawIncrementalLetters(wxCommandEvent &event)
{
  wxLogDebug("Canvas: OnDrawIncrementalLetters");
}
void Canvas::OnDrawArrow(wxCommandEvent &event)
{
  wxLogDebug("Canvas: OnDrawArrow");
}
void Canvas::OnDrawCircle(wxCommandEvent &event)
{
  wxLogDebug("Canvas: OnDrawCircle");
}
void Canvas::OnDrawLine(wxCommandEvent &event)
{
  wxLogDebug("Canvas: OnDrawLine");
}
void Canvas::OnDrawMultiline(wxCommandEvent &event)
{
  wxLogDebug("Canvas: OnDrawMultiline");
}
void Canvas::OnDrawRectangle(wxCommandEvent &event)
{
  wxLogDebug("Canvas: OnDrawRectangle");
}
void Canvas::OnDrawSkinTemplate(wxCommandEvent &event)
{ 
  wxLogDebug("Canvas: OnDrawSkinTemplate");
}
}
