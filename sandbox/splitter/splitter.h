#pragma once

#include <wx/wx.h>
#include <wx/event.h>

wxDECLARE_EVENT(EVT_MY_BUTTON_EVENT, wxCommandEvent);

class LeftPanel : public wxScrolled<wxWindow>
{
public:
  LeftPanel(wxWindow* parent);
  void OnCharInput(wxKeyEvent& event);
private:
  wxBoxSizer* m_sizer;
};

class RightPanel : public wxScrolled<wxWindow>
{
public:
  RightPanel(wxWindow* parent);
};
