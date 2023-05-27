#pragma once

#include <wx/panel.h>
#include <vector>

#include "bitmappane.h"
#include "toolbox_events.h"

namespace app
{

class Toolbox : public wxPanel
{
public:
  Toolbox(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
  ~Toolbox() = default;

  void OnScreenshot();
  void OnSave();
  void OnDelete();
  void OnUndo();
  
  void OnDrawText();
  void OnDrawIncrementalLetters();
  void OnDrawArrow();
  void OnDrawCircle();
  void OnDrawLine();
  void OnDrawMultiline();
  void OnDrawRectangle();
  void OnDrawSkinTemplate();
private:

  void SelectPane(BitmapPane* pane);
  std::vector<BitmapPane*> m_colorPanes;
  using ButtonHandler = std::pair<wxBitmap, std::function<void()>>;
  std::vector<ButtonHandler> m_buttonHandlers;
  const std::string lightBackground = "#f4f3f3";
  const std::string darkBackground = "#2c2828";
};

}