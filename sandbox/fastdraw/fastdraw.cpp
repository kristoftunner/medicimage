#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/image.h>
#include <wx/graphics.h>
#include <wx/dcbuffer.h>
#include <wx/custombgwin.h>
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

  void DrawBitmap(wxBitmap& bitmap)
  {
    wxMemoryDC memDC(bitmap);
    auto gc = wxGraphicsContext::Create(memDC);
    if(gc)
    {
      
      gc->SetPen(*wxWHITE_PEN);
      gc->SetBrush(*wxWHITE_BRUSH);
      gc->DrawRectangle(0, 0, 1280, 780);
      auto width = m_secondPoint.x - m_firstPoint.x;
      auto height = m_secondPoint.y - m_firstPoint.y;
      gc->SetPen(*wxRED_PEN);
      gc->DrawRectangle(m_firstPoint.x , m_firstPoint.y , width, height);
      delete gc;
    }
    memDC.SelectObject(wxNullBitmap);
  }
  
  void OnPaint(wxPaintEvent& event)
  {
  #if 1
    wxBitmap bitmap(1280, 720);
    DrawBitmap(bitmap);

    wxPaintDC dc(this);
    dc.Clear();
    dc.SetUserScale(.5, .5);
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
 
class MyFrame : public wxCustomBackgroundWindow<wxFrame>
{
public:
  MyFrame() 
  {
    Create(nullptr, wxID_ANY, "FastDraw");
      wxImage image("wallpaper.png", wxBITMAP_TYPE_PNG);
      if (image.IsOk())
      {
        image.SetAlpha();
        for(auto y = 0; y < image.GetHeight(); ++y)
        {
          for(auto x = 0; x < image.GetWidth(); ++x)
          {
            image.SetAlpha(x, y, 128);
          }
        }
        wxBitmap bitmap(image);
        SetBackgroundBitmap(bitmap);
      }
    //SetBackgroundStyle(wxBG_STYLE_PAINT);
    auto sizer = new wxBoxSizer(wxVERTICAL);
    //auto canvas = new DrawingPanel(this);
    auto panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(800, 600));
    panel->SetBackgroundStyle(wxBG_STYLE_TRANSPARENT);
    panel->SetBackgroundColour(wxColor(0, 0, 0, 0));
    panel->SetTransparent(255);
    auto panelSizer = new wxBoxSizer(wxVERTICAL);
    auto text = new wxStaticText(this, wxID_ANY, "Hello World");
    text->SetTransparent(128);
    auto button = new wxButton(this, wxID_ANY, "Click Me");
    button->SetTransparent(128);
    panelSizer->Add(text, wxSizerFlags(0));
    panelSizer->Add(button, wxSizerFlags(0));
    sizer->Add(panelSizer, wxSizerFlags(0));
    //Bind(wxEVT_PAINT, &MyFrame::OnPaint, this);
    Bind(wxEVT_LEFT_DOWN, &MyFrame::OnMousePressed, this);
    Bind(wxEVT_LEFT_UP, &MyFrame::OnMouseReleased, this);
    Bind(wxEVT_MOTION, &MyFrame::OnMouseMoved, this);

    SetSizer(sizer);
  }
  void OnPaint(wxPaintEvent& event)
  {
    wxPaintDC dc(this);
    //dc.Clear();
    auto gc = wxGraphicsContext::Create(dc);
    if(gc)
    {
      auto width = m_secondPoint.x - m_firstPoint.x;
      auto height = m_secondPoint.y - m_firstPoint.y;
      gc->SetPen(*wxRED_PEN);
      gc->SetBrush(*wxTRANSPARENT_BRUSH);
      gc->DrawRectangle(m_firstPoint.x , m_firstPoint.y , width, height);
      delete gc;
    }
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

private:
  wxPoint m_firstPoint;
  wxPoint m_secondPoint;
  bool m_mouseDown = false;
};
 
bool MyApp::OnInit()
{
  wxInitAllImageHandlers();  
  auto frame = new MyFrame();
  frame->SetSize(0, 0, 800, 600);
  frame->Show(true);
  return true;
}

} // namespace app
