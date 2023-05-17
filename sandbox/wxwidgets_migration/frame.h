#pragma once

#include <wx/gbsizer.h>
#include <wx/frame.h>
#include <wx/panel.h>
#include <wx/log.h>
#include <string>

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

  wxPanel* m_panel;
  wxGridBagSizer* m_gbs;
  wxGBPosition m_lastPos;
  wxLog* m_logger;

  const std::string m_lightBackground = "#f4f3f3";
  const std::string m_darkBackground = "#2c2828";

};

}
