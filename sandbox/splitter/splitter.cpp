#include <wx/wx.h>
#include <wx/splitter.h>

class MyFrame : public wxFrame
{
public:
    MyFrame(const wxString& title)
        : wxFrame(NULL, wxID_ANY, title)
    {
        // Create a splitter window
        wxSplitterWindow* splitter = new wxSplitterWindow(this, wxID_ANY, wxPoint(-1,-1) ,wxSize(-1,-1),wxSP_LIVE_UPDATE);

        // Create panels for the splitter panes
        wxPanel* panel1 = new wxPanel(splitter, wxID_ANY);
        wxPanel* panel2 = new wxPanel(splitter, wxID_ANY);
        panel1->SetBackgroundColour(*wxRED);
        panel2->SetBackgroundColour(*wxBLUE);
        // Set the splitter window's panes
        splitter->SplitVertically(panel1, panel2);

        // Set the minimum pane sizes
        splitter->SetMinimumPaneSize(200);

        // Set the splitter window as the main window
        SetSizer(new wxBoxSizer(wxVERTICAL));
        GetSizer()->Add(splitter, 1, wxEXPAND);
    }
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
