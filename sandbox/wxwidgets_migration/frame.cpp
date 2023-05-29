#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/log.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/splitter.h>

#include "frame.h"
#include "thumbnails/thumbnails.h"
#include "toolbox/toolbox.h"
#include "toolbox/attribute_editor.h"
#include "toolbox/toolbox_events.h"
#include "editor/canvas.h"
#include "editor/editor.h"
#include "toolbox/attribute_editor_events.h"

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
  
  bool isDark = wxSystemSettings::GetAppearance().IsDark();
  const auto margin = FromDIP(5);
  
  auto mainSplitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE | wxSP_3DSASH);
  auto nestedSplitter = new wxSplitterWindow(mainSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE | wxSP_3DSASH);
  mainSplitter->SetSashSize(FromDIP(10));
  nestedSplitter->SetSashSize(FromDIP(10));

  auto toolbox = new Toolbox(nestedSplitter, wxID_ANY);
  auto editorPanel = new EditorPanel(nestedSplitter, wxID_ANY);
  
  nestedSplitter->SplitVertically(toolbox, editorPanel, 200);
  nestedSplitter->SetMinimumPaneSize(200);
  nestedSplitter->SetDoubleBuffered(true);
  mainSplitter->SetSashGravity(0.5);
  
  auto thumbnails = new Thumbnails(mainSplitter, wxID_ANY);
  thumbnails->SetBackgroundColour(wxColour(isDark ? m_darkBackground : m_lightBackground));
  
  mainSplitter->SplitVertically(nestedSplitter, thumbnails, 400);
  mainSplitter->SetMinimumPaneSize(200);
  mainSplitter->SetDoubleBuffered(true);
  mainSplitter->SetSashGravity(0.8);

  wxBoxSizer *topsizer = new wxBoxSizer( wxHORIZONTAL );
  topsizer->Add(mainSplitter, 1, wxEXPAND );
  
  Bind(wxEVT_MENU, &MyFrame::OnHello, this, ID_Hello);
  Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
  Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);

  // Events generated by toolbox and relayed to canvas
  auto relayToolboxEvent = [this, editorPanel](wxCommandEvent &event){wxPostEvent(editorPanel, event);};
  Bind(TOOLBOX_SCREENSHOT, relayToolboxEvent);
  Bind(TOOLBOX_SAVE, relayToolboxEvent);
  Bind(TOOLBOX_DELETE, relayToolboxEvent);
  Bind(TOOLBOX_UNDO, relayToolboxEvent);

  Bind(TOOLBOX_DRAW_TEXT, relayToolboxEvent);
  Bind(TOOLBOX_DRAW_LETTERS, relayToolboxEvent);
  Bind(TOOLBOX_DRAW_ARROW, relayToolboxEvent);
  Bind(TOOLBOX_DRAW_CIRCLE, relayToolboxEvent);
  Bind(TOOLBOX_DRAW_LINE, relayToolboxEvent);
  Bind(TOOLBOX_DRAW_MULTILINE, relayToolboxEvent);
  Bind(TOOLBOX_DRAW_RECTANGLE, relayToolboxEvent);
  Bind(TOOLBOX_DRAW_SKIN_TEMPLATE, relayToolboxEvent);

  // Events generated by the attribute editor and relayed to the editor
  Bind(EVT_ENTITY_ATTRIBUTE_EDITED, [this, editorPanel](EntityEvent &event){
    wxPostEvent(editorPanel, event);}
  );
  // Events generated by canvas/editorpanel and relayed to thumbnails
  auto relayCanvasEvent = [this, thumbnails](ImageDocumentEvent &event){wxPostEvent(thumbnails, event);};
  Bind(EVT_EDITOR_DELETE_DOCUMENT, relayCanvasEvent);
  Bind(EVT_EDITOR_SAVE_DOCUMENT, relayCanvasEvent);
  Bind(EVT_EDITOR_ADD_DOCUMENT, relayCanvasEvent);
  Bind(EVT_THUMBNAILS_ADD_PATIENT, [this, thumbnails](PatientEvent &event){wxPostEvent(thumbnails, event);});
  Bind(EVT_EDITOR_ENTITY_CHANGED, [this, toolbox](EntityEvent &event){wxPostEvent(toolbox, event);});

  // Events generated by thumbnails and relayed to canvas
  auto relayThumbnailEvent = [this, editorPanel](ImageDocumentEvent &event){
    wxPostEvent(editorPanel, event);
  };
  Bind(EVT_THUMBNAILS_DOCUMENT_PICK, relayThumbnailEvent);
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