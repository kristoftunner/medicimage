#pragma once

#include <wx/gbsizer.h>
#include <wx/frame.h>
#include <wx/panel.h>
#include <wx/log.h>
#include <string>
#include <wx/print.h>

#include "editor/canvas.h"

namespace app
{

class MyFrame : public wxFrame
{
public:
  MyFrame();
private:
  void OnHello(wxCommandEvent& event);
  void OnExit(wxCommandEvent& event);
  void OnAbout(wxCommandEvent& event);
  void OnPrint(wxCommandEvent& event);

  wxPanel* m_panel;
  wxGridBagSizer* m_gbs;
  wxGBPosition m_lastPos;
  wxLog* m_logger;
  EditorPanel* m_editorPanel;

  const std::string m_lightBackground = "#f4f3f3";
  const std::string m_darkBackground = "#2c2828";

};

class Printout: public wxPrintout
{
public:
  Printout(Canvas* canvas, const wxString &title = "My printout")
      : wxPrintout(title) { m_canvas=canvas; }

  virtual bool OnPrintPage(int page) wxOVERRIDE;
  virtual bool HasPage(int page) wxOVERRIDE;
  virtual bool OnBeginDocument(int startPage, int endPage) wxOVERRIDE;
  virtual void GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo) wxOVERRIDE;

  void DrawPage();

private:
  Canvas *m_canvas;
};
}
