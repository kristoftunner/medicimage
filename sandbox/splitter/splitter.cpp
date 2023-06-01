#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/clrpicker.h>

#include "splitter.h"

wxDEFINE_EVENT(EVT_MY_BUTTON_EVENT, wxCommandEvent);

LeftPanel::LeftPanel(wxWindow* parent)
  : wxScrolled<wxWindow>(parent, wxID_ANY)
{
  m_sizer = new wxBoxSizer(wxVERTICAL);
  
  SetSizer(m_sizer);
  SetScrollRate(FromDIP(5), FromDIP(5));
  SetVirtualSize(FromDIP(600), FromDIP(400));
  Bind(EVT_MY_BUTTON_EVENT, [this](wxCommandEvent& event) {
    for(int i = 0; i < 20; ++i)
      m_sizer->Add(new wxButton(this, wxID_ANY, "Button" + std::to_string(i)), 0, wxEXPAND);
    wxColourPickerCtrl* colorPicker = new wxColourPickerCtrl(this, wxID_ANY, wxColor(255,0,0), wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE);
    m_sizer->Add(colorPicker, 0, wxEXPAND);
    auto size = m_sizer->CalcMin();
    SetVirtualSize(FromDIP(size.GetWidth()), FromDIP(size.GetHeight()));
    m_sizer->Layout();
    Refresh();
  });
}

void LeftPanel::OnCharInput(wxKeyEvent& event)
{
  wxLogDebug("MyPanel::OnCharInput");
  event.Skip();
};


RightPanel::RightPanel(wxWindow* parent)
  : wxScrolled<wxWindow>(parent, wxID_ANY)
{
  wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
  
  SetSizer(sizer);
  auto button = new wxButton(this, wxID_ANY, "Button");
  sizer->Add(button, 0, wxEXPAND);
  button->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event) {
    wxLogDebug("Button clicked");
    wxCommandEvent event2(EVT_MY_BUTTON_EVENT);
    this->ProcessWindowEvent(event2);
  });

  SetScrollRate(FromDIP(5), FromDIP(5));
  SetVirtualSize(FromDIP(600), FromDIP(400));
}

class MyFrame : public wxFrame
{
public:
    MyFrame(const wxString& title)
        : wxFrame(NULL, wxID_ANY, title)
    {
        // Create a splitter window
        wxSplitterWindow* splitter = new wxSplitterWindow(this, wxID_ANY, wxPoint(-1,-1) ,wxSize(-1,-1),wxSP_LIVE_UPDATE);

        // Create panels for the splitter panes
        auto leftPanel = new LeftPanel(splitter);
        auto rightPanel = new RightPanel(splitter);
        leftPanel->SetBackgroundColour(*wxRED);
        rightPanel->SetBackgroundColour(*wxBLUE);
        // Set the splitter window's panes
        splitter->SplitVertically(leftPanel, rightPanel);

        // Set the minimum pane sizes
        splitter->SetMinimumPaneSize(200);

        // Set the splitter window as the main window
        SetSizer(new wxBoxSizer(wxVERTICAL));
        GetSizer()->Add(splitter, 1, wxEXPAND);
        
        Bind(EVT_MY_BUTTON_EVENT, [this, leftPanel](wxCommandEvent& event) {
          wxPostEvent(leftPanel, event);
        });
        
    }
  void OnCharInput(wxKeyEvent& event)
  {
    wxLogDebug("MyPanel::OnCharInput");
    event.Skip();
  };
};

class MyApp : public wxApp
{
public:
    virtual bool OnInit()
    {
        MyFrame* frame = new MyFrame("Splitter Window Example");
        frame->Show();
        return true;
    }
};

IMPLEMENT_APP(MyApp);
