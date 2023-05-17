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
    if(colorPane->selected)
      wxLogDebug("Colorpane selected");
    colorPane->Refresh();
  }
}

void Toolbox::OnScreenshot()
{
  wxLogDebug("OnxScreenshot");
}

void Toolbox::OnSave()
{
  wxLogDebug("OnSave");
}

void Toolbox::OnDelete()
{
  wxLogDebug("OnDelete");
}

void Toolbox::OnUndo()
{
  wxLogDebug("OnUndo");
}

void Toolbox::OnDrawText()
{
  wxLogDebug("OnDrawText");
}

void Toolbox::OnDrawIncrementalLetters()
{
  wxLogDebug("OnDrawIncrementalLetters");
}

void Toolbox::OnDrawArrow()
{
  wxLogDebug("OnDrawArrow");
}

void Toolbox::OnDrawCircle()
{
  wxLogDebug("OnDrawCircle");
}

void Toolbox::OnDrawLine()
{
  wxLogDebug("OnDrawLine");
}

void Toolbox::OnDrawMultiline()
{
  wxLogDebug("OnDrawMultiline");
}

void Toolbox::OnDrawRectangle()
{
  wxLogDebug("OnDrawRectangle");
}

void Toolbox::OnDrawSkinTemplate()
{
  wxLogDebug("OnDrawSkinTemplate");
}

}