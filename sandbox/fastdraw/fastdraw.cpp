#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/image.h>
#include <wx/graphics.h>
#include <wx/dcbuffer.h>
#include <glm/glm.hpp>

namespace app
{

class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};
 
wxIMPLEMENT_APP(MyApp);

class DrawingPanel : public wxPanel
{
public:
  DrawingPanel(wxFrame* parent) : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(800, 600))
  {
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    Bind(wxEVT_PAINT, &DrawingPanel::OnPaint, this);
    Bind(wxEVT_LEFT_DOWN, &DrawingPanel::OnMousePressed, this);
    Bind(wxEVT_LEFT_UP, &DrawingPanel::OnMouseReleased, this);
    Bind(wxEVT_MOTION, &DrawingPanel::OnMouseMoved, this);
  }

  void OnMouseMoved(wxMouseEvent &event)
  {
    if(m_mouseDown)
    {
      wxClientDC dc(this);
      PrepareDC(dc);
      m_secondPoint = wxPoint(event.GetLogicalPosition(dc));
      Refresh();
    }
  }

  void OnMousePressed(wxMouseEvent &event)
  {
    wxClientDC dc(this);
    PrepareDC(dc);
    m_firstPoint = wxPoint(event.GetLogicalPosition(dc));
    Refresh();
    m_mouseDown = true;
  }

  void OnMouseReleased(wxMouseEvent &event)
  {
    wxClientDC dc(this);
    PrepareDC(dc);
    m_secondPoint = wxPoint(event.GetLogicalPosition(dc));
    Refresh();
    m_mouseDown = false;
  }

  void OnPaint(wxPaintEvent& event)
  {
  #if 1
    wxBitmap bitmap(400,600);
    wxMemoryDC memDC(bitmap);
    auto gc = wxGraphicsContext::Create(memDC);
    if(gc)
    {
      
      gc->SetPen(*wxWHITE_PEN);
      gc->SetBrush(*wxWHITE_BRUSH);
      gc->DrawRectangle(0, 0, 400, 600);
      auto width = m_secondPoint.x - m_firstPoint.x;
      auto height = m_secondPoint.y - m_firstPoint.y;
      gc->SetPen(*wxRED_PEN);
      gc->DrawRectangle(m_firstPoint.x, m_firstPoint.y, width, height);
      delete gc;
    }

    memDC.SelectObject(wxNullBitmap);

    wxPaintDC dc(this);
    dc.DrawBitmap(bitmap, 0, 0);
#else
    wxPaintDC dc(this);
    auto gc = wxGraphicsContext::Create(dc);
    if(gc)
    {
      
      gc->SetPen(*wxWHITE_PEN);
      gc->SetBrush(*wxWHITE_BRUSH);
      gc->DrawRectangle(0, 0, 400, 600);
      auto width = m_secondPoint.x - m_firstPoint.x;
      auto height = m_secondPoint.y - m_firstPoint.y;
      gc->SetPen(*wxRED_PEN);
      gc->DrawRectangle(m_firstPoint.x, m_firstPoint.y, width, height);
      delete gc;
    }
#endif

  }

private:
  wxPoint m_firstPoint;
  wxPoint m_secondPoint;
  bool m_mouseDown = false;
};
 
class MyFrame : public wxFrame
{
public:
  MyFrame() : wxFrame(nullptr, wxID_ANY, "FastDraw")
  {
    auto sizer = new wxBoxSizer(wxVERTICAL);
    auto canvas = new DrawingPanel(this);
    sizer->Add(canvas, 1, wxEXPAND);
    SetSizer(sizer);
  }
};
 
bool MyApp::OnInit()
{
  auto frame = new MyFrame();
  frame->SetSize(0, 0, 800, 600);
  frame->Show(true);
  return true;
}

} // namespace app
