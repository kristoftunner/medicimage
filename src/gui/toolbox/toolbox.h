#pragma once

#include <wx/panel.h>
#include <vector>
#include <unordered_map>

#include "bitmappane.h"
#include "toolbox.h"
#include "attribute_editor.h"
#include "gui/custom_events.h"

namespace app
{

struct BitmapButton
{
  BitmapPane* pane;
  std::function<void()> handlerFn;
};

class Toolbox : public wxPanel
{
public:
  Toolbox(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
  ~Toolbox() = default;

  void OnScreenshot();
  void OnSave();
  void OnDelete();
  void OnUndo();
  void OnCancel();
  
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
  std::unordered_map<ButtonType, BitmapButton> m_buttons;
  AttributeEditor* m_attributeEditor = nullptr;

  const std::string m_lightBackground = "#f4f3f3";
  const std::string m_darkBackground = "#2c2828";
  static constexpr int s_buttonSize = 60;
};

}