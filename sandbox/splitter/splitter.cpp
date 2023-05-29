#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/clrpicker.h>
class MyPanel : public wxScrolled<wxWindow>
{
public:
  MyPanel(wxWindow* parent)
    : wxScrolled<wxWindow>(parent, wxID_ANY)
  {
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    
    SetSizer(sizer);
    for(int i = 0; i < 2; ++i)
      sizer->Add(new wxButton(this, wxID_ANY, "Button" + std::to_string(i)), 0, wxEXPAND);
    wxColourPickerCtrl* colorPicker = new wxColourPickerCtrl(this, wxID_ANY, wxColor(255,0,0), wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE);
    sizer->Add(colorPicker, 0, wxEXPAND);
    SetScrollRate(FromDIP(5), FromDIP(5));
    SetVirtualSize(FromDIP(600), FromDIP(400));
    Bind(wxEVT_CHAR_HOOK, &MyPanel::OnCharInput, this);
  }
  void OnCharInput(wxKeyEvent& event)
  {
    wxLogDebug("MyPanel::OnCharInput");
    event.Skip();
  };
};


class MyFrame : public wxFrame
{
public:
    MyFrame(const wxString& title)
        : wxFrame(NULL, wxID_ANY, title)
    {
        // Create a splitter window
        wxSplitterWindow* splitter = new wxSplitterWindow(this, wxID_ANY, wxPoint(-1,-1) ,wxSize(-1,-1),wxSP_LIVE_UPDATE);

        // Create panels for the splitter panes
        auto panel1 = new MyPanel(splitter);
        wxPanel* panel2 = new wxPanel(splitter);
        panel1->SetBackgroundColour(*wxRED);
        panel2->SetBackgroundColour(*wxBLUE);
        // Set the splitter window's panes
        splitter->SplitVertically(panel1, panel2);

        // Set the minimum pane sizes
        splitter->SetMinimumPaneSize(200);

        // Set the splitter window as the main window
        SetSizer(new wxBoxSizer(wxVERTICAL));
        GetSizer()->Add(splitter, 1, wxEXPAND);
        //Bind(wxEVT_CHAR_HOOK, &MyFrame::OnCharInput, this);
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
