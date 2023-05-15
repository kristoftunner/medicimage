#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/log.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/button.h>

#include "frame.h"
#include "thumbnails.h"
#include "canvas.h"
namespace app
{

enum
{
    ID_Hello = 1
};

MyFrame::MyFrame()
  : wxFrame(NULL, wxID_ANY, "Hello World")
{
  m_logger = new wxLogWindow(this, "Log", true, false);
  wxLog::SetActiveTarget(m_logger);

  
  wxMenu *menuFile = new wxMenu;
  menuFile->Append(ID_Hello, "&Hello...\tCtrl-H",
                   "Help string shown in status bar for this menu item");
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT);
  wxMenu *menuHelp = new wxMenu;
  menuHelp->Append(wxID_ABOUT);
  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(menuFile, "&File");
  menuBar->Append(menuHelp, "&Help");
  SetMenuBar( menuBar );

  const auto margin = FromDIP(5);
  auto* toolbox = new wxPanel(this, wxID_ANY);
  toolbox->SetBackgroundColour(wxColour(100, 200, 100));

  auto gridBagLayout = new Thumbnails(this, wxID_ANY);
  auto canvas = new Canvas(this, wxID_ANY);
  
  wxBoxSizer *topsizer = new wxBoxSizer( wxHORIZONTAL );
  topsizer->Add(toolbox, 1, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, margin);
  topsizer->Add(canvas,1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, margin);
  topsizer->Add(gridBagLayout,1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, margin);
  
  this->SetSizerAndFit(topsizer);
  Bind(wxEVT_MENU, &MyFrame::OnHello, this, ID_Hello);
  Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
  Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
}
 
void MyFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}
 
void MyFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("This is a wxWidgets Hello World example",
                 "About Hello World", wxOK | wxICON_INFORMATION);
}
 
void MyFrame::OnHello(wxCommandEvent& event)
{
    wxLogMessage("Hello world from wxWidgets!");
}

}