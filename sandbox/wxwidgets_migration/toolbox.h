#pragma once

#include <wx/panel.h>

namespace app
{

class Toolbox : public wxPanel
{
public:
  Toolbox(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
  ~Toolbox() = default;

  void OnScreenshot(wxCommandEvent& event);
  void OnSave(wxCommandEvent& event);
  void OnDelete(wxCommandEvent& event);
  void OnUndo(wxCommandEvent& event);
  
  void OnDrawText(wxCommandEvent& event);
  void OnDrawIncrementalLetters(wxCommandEvent& event);
  void OnDrawArrow(wxCommandEvent& event);
  void OnDrawCircle(wxCommandEvent& event);
  void OnDrawLine(wxCommandEvent& event);
  void OnDrawMultiline(wxCommandEvent& event);
  void OnDrawRectangle(wxCommandEvent& event);
  void OnDrawSkinTemplate(wxCommandEvent& event);
};

}