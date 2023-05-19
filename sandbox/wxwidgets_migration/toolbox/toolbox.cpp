#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/log.h>
#include <wx/wx.h>
#include <wx/settings.h>
#include <wx/wrapsizer.h>
#include <wx/bitmap.h>

#include "toolbox.h"

namespace app
{

wxDEFINE_EVENT(TOOLBOX_SCREENSHOT, wxCommandEvent);
wxDEFINE_EVENT(TOOLBOX_SAVE, wxCommandEvent);
wxDEFINE_EVENT(TOOLBOX_DELETE, wxCommandEvent);
wxDEFINE_EVENT(TOOLBOX_UNDO, wxCommandEvent);

wxDEFINE_EVENT(TOOLBOX_DRAW_TEXT, wxCommandEvent);
wxDEFINE_EVENT(TOOLBOX_DRAW_LETTERS, wxCommandEvent);
wxDEFINE_EVENT(TOOLBOX_DRAW_ARROW, wxCommandEvent);
wxDEFINE_EVENT(TOOLBOX_DRAW_CIRCLE, wxCommandEvent);
wxDEFINE_EVENT(TOOLBOX_DRAW_LINE, wxCommandEvent);
wxDEFINE_EVENT(TOOLBOX_DRAW_MULTILINE, wxCommandEvent);
wxDEFINE_EVENT(TOOLBOX_DRAW_RECTANGLE, wxCommandEvent);
wxDEFINE_EVENT(TOOLBOX_DRAW_SKIN_TEMPLATE, wxCommandEvent);

Toolbox::Toolbox(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size)
  : wxPanel(parent, id, pos, size)
{
  const std::vector<std::string> niceColors = {"#000000", "#ffffff", "#fd7f6f",
                                             "#ffb55a", "#ffee65", "#beb9db",
                                               "#fdcce5", "#8bd3c7"};
  m_buttonHandlers.emplace_back(wxBitmap("screenshot.png", wxBITMAP_TYPE_ANY), [this]() { OnScreenshot(); });
  m_buttonHandlers.emplace_back(wxBitmap("save.png", wxBITMAP_TYPE_ANY), [this]() { OnSave(); });
  m_buttonHandlers.emplace_back(wxBitmap("delete.png", wxBITMAP_TYPE_ANY), [this]() { OnDelete(); });
  m_buttonHandlers.emplace_back(wxBitmap("undo.png", wxBITMAP_TYPE_ANY), [this]() { OnUndo(); }); 
  m_buttonHandlers.emplace_back(wxBitmap("add-text.png", wxBITMAP_TYPE_ANY), [this]() { OnDrawText(); });
  m_buttonHandlers.emplace_back(wxBitmap("add-incremental-letters.png", wxBITMAP_TYPE_ANY), [this]() { OnDrawIncrementalLetters(); });
  m_buttonHandlers.emplace_back(wxBitmap("arrow.png", wxBITMAP_TYPE_ANY), [this]() { OnDrawArrow(); });
  m_buttonHandlers.emplace_back(wxBitmap("circle.png", wxBITMAP_TYPE_ANY), [this]() { OnDrawCircle(); });
  m_buttonHandlers.emplace_back(wxBitmap("line.png", wxBITMAP_TYPE_ANY), [this]() { OnDrawLine(); });
  m_buttonHandlers.emplace_back(wxBitmap("multiline.png", wxBITMAP_TYPE_ANY), [this]() { OnDrawMultiline(); });
  m_buttonHandlers.emplace_back(wxBitmap("rectangle.png", wxBITMAP_TYPE_ANY), [this]() { OnDrawRectangle(); });
  m_buttonHandlers.emplace_back(wxBitmap("skin-template.png", wxBITMAP_TYPE_ANY), [this]() { OnDrawSkinTemplate(); });

  auto topSizer = new wxBoxSizer(wxVERTICAL);
  auto text = new wxStaticText(this, wxID_ANY, "Toolbox");
  topSizer->Add(text, 0, wxALL, FromDIP(5));

  auto toolboxPaneSizer = new wxWrapSizer(wxHORIZONTAL);
  for(auto& buttonHandler : m_buttonHandlers)
  {
    auto pane = new BitmapPane(buttonHandler.first, this, wxID_ANY, wxColour(lightBackground));
    pane->Bind(wxEVT_LEFT_DOWN, [this, pane, buttonHandler](wxMouseEvent &event)
    {
      SelectPane(pane);
      buttonHandler.second();
    });
    m_colorPanes.push_back(pane);
    toolboxPaneSizer->Add(pane, 0, wxALL, FromDIP(5));
  }
  topSizer->Add(toolboxPaneSizer, 0, wxALL, FromDIP(5));

  SetSizer(topSizer);
}

void Toolbox::SelectPane(BitmapPane* pane)
{
  for (auto colorPane : m_colorPanes)
  {
    colorPane->selected = (colorPane == pane);
    colorPane->Refresh();
  }
}

void Toolbox::OnScreenshot()
{
  wxCommandEvent event(TOOLBOX_SCREENSHOT, GetId());
  event.SetEventObject(this);
  ProcessWindowEvent(event);
  wxLogDebug("OnxScreenshot");
}

void Toolbox::OnSave()
{
  wxCommandEvent event(TOOLBOX_SAVE, GetId());
  event.SetEventObject(this);
  ProcessWindowEvent(event);
  wxLogDebug("OnSave");
}

void Toolbox::OnDelete()
{
  wxCommandEvent event(TOOLBOX_DELETE, GetId());
  event.SetEventObject(this);
  ProcessWindowEvent(event);
  wxLogDebug("OnDelete");
}

void Toolbox::OnUndo()
{
  wxCommandEvent event(TOOLBOX_UNDO, GetId());
  event.SetEventObject(this);
  ProcessWindowEvent(event);
  wxLogDebug("OnUndo");
}

void Toolbox::OnDrawText()
{
  wxCommandEvent event(TOOLBOX_DRAW_TEXT, GetId());
  event.SetEventObject(this);
  ProcessWindowEvent(event);
  wxLogDebug("OnDrawText");
}

void Toolbox::OnDrawIncrementalLetters()
{
  wxCommandEvent event(TOOLBOX_DRAW_LETTERS, GetId());
  event.SetEventObject(this);
  ProcessWindowEvent(event);
  wxLogDebug("OnDrawIncrementalLetters");
}

void Toolbox::OnDrawArrow()
{
  wxCommandEvent event(TOOLBOX_DRAW_ARROW, GetId());
  event.SetEventObject(this);
  ProcessWindowEvent(event);
  wxLogDebug("OnDrawArrow");
}

void Toolbox::OnDrawCircle()
{
  wxCommandEvent event(TOOLBOX_DRAW_CIRCLE, GetId());
  event.SetEventObject(this);
  ProcessWindowEvent(event);
  wxLogDebug("OnDrawCircle");
}

void Toolbox::OnDrawLine()
{
  wxCommandEvent event(TOOLBOX_DRAW_LINE, GetId());
  event.SetEventObject(this);
  ProcessWindowEvent(event);
  wxLogDebug("OnDrawLine");
}

void Toolbox::OnDrawMultiline()
{
  wxCommandEvent event(TOOLBOX_DRAW_MULTILINE, GetId());
  event.SetEventObject(this);
  ProcessWindowEvent(event);
  wxLogDebug("OnDrawMultiline");
}

void Toolbox::OnDrawRectangle()
{
  wxCommandEvent event(TOOLBOX_DRAW_RECTANGLE, GetId());
  event.SetEventObject(this);
  ProcessWindowEvent(event);
  wxLogDebug("OnDrawRectangle");
}

void Toolbox::OnDrawSkinTemplate()
{
  wxCommandEvent event(TOOLBOX_DRAW_SKIN_TEMPLATE, GetId());
  event.SetEventObject(this);
  ProcessWindowEvent(event);
  wxLogDebug("OnDrawSkinTemplate");
}

}