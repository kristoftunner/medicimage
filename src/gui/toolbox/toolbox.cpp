#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/log.h>
#include <wx/wx.h>
#include <wx/settings.h>
#include <wx/wrapsizer.h>
#include <wx/bitmap.h>
#include <wx/debug.h>

#include "toolbox.h"
#include "toolbox_events.h"
#include "gui/editor/editor_events.h"

namespace app
{

  Toolbox::Toolbox(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
    : wxPanel(parent, id, pos, size)
  {
    m_buttons.insert({ ButtonType::SCREENSHOT_BUTTON, 
      BitmapButton(new BitmapPane(wxBitmap("screenshot.png", wxBITMAP_TYPE_PNG), this, wxID_ANY, wxColour(s_lightBackground), true, wxDefaultPosition, {s_buttonSize, s_buttonSize}), [this]() { OnScreenshot(); }) });
    m_buttons.insert({ ButtonType::SAVE_BUTTON,   
      BitmapButton(new BitmapPane(wxBitmap("save.png", wxBITMAP_TYPE_PNG), this, wxID_ANY, wxColour(s_lightBackground), true, wxDefaultPosition, {s_buttonSize, s_buttonSize}), [this]() { OnSave(); }) });
    m_buttons.insert({ ButtonType::DELETE_BUTTON, 
      BitmapButton(new BitmapPane(wxBitmap("delete.png", wxBITMAP_TYPE_PNG), this, wxID_ANY, wxColour(s_lightBackground), true, wxDefaultPosition, {s_buttonSize, s_buttonSize}), [this]() { OnDelete(); }) });
    m_buttons.insert({ ButtonType::UNDO_BUTTON, 
      BitmapButton(new BitmapPane(wxBitmap("undo.png", wxBITMAP_TYPE_PNG), this, wxID_ANY, wxColour(s_lightBackground), true, wxDefaultPosition, {s_buttonSize, s_buttonSize}), [this]() { OnUndo(); }) });
    m_buttons.insert({ ButtonType::CANCEL_BUTTON, 
      BitmapButton(new BitmapPane(wxBitmap("cancel.png", wxBITMAP_TYPE_PNG), this, wxID_ANY, wxColour(s_lightBackground), true, wxDefaultPosition, {s_buttonSize, s_buttonSize}), [this]() { OnCancel(); }) });
    m_buttons.insert({ ButtonType::DRAW_TEXT_BUTTON, 
      BitmapButton(new BitmapPane(wxBitmap("add-text.png", wxBITMAP_TYPE_PNG), this, wxID_ANY, wxColour(s_lightBackground), true, wxDefaultPosition, {s_buttonSize, s_buttonSize}), [this]() { OnDrawText(); }) });
    m_buttons.insert({ ButtonType::DRAW_LETTERS_BUTTON, 
      BitmapButton(new BitmapPane(wxBitmap("add-incremental-letters.png", wxBITMAP_TYPE_PNG), this, wxID_ANY, wxColour(s_lightBackground), true, wxDefaultPosition, {s_buttonSize, s_buttonSize}), [this]() { OnDrawIncrementalLetters(); }) });
    m_buttons.insert({ ButtonType::DRAW_ARROW_BUTTON,  
      BitmapButton(new BitmapPane(wxBitmap("arrow.png", wxBITMAP_TYPE_PNG), this, wxID_ANY, wxColour(s_lightBackground), true, wxDefaultPosition, {s_buttonSize, s_buttonSize}), [this]() { OnDrawArrow(); }) });
    m_buttons.insert({ ButtonType::DRAW_CIRCLE_BUTTON, 
      BitmapButton(new BitmapPane(wxBitmap("circle.png", wxBITMAP_TYPE_PNG), this, wxID_ANY, wxColour(s_lightBackground), true, wxDefaultPosition, {s_buttonSize, s_buttonSize}), [this]() { OnDrawCircle(); }) });
    m_buttons.insert({ ButtonType::DRAW_LINE_BUTTON, 
      BitmapButton(new BitmapPane(wxBitmap("line.png", wxBITMAP_TYPE_PNG), this, wxID_ANY, wxColour(s_lightBackground), true, wxDefaultPosition, {s_buttonSize, s_buttonSize}), [this]() { OnDrawLine(); }) });
    m_buttons.insert({ ButtonType::DRAW_MULTILINE_BUTTON, 
      BitmapButton(new BitmapPane(wxBitmap("multiline.png", wxBITMAP_TYPE_PNG), this, wxID_ANY, wxColour(s_lightBackground), true, wxDefaultPosition, {s_buttonSize, s_buttonSize}), [this]() { OnDrawMultiline(); }) });
    m_buttons.insert({ ButtonType::DRAW_RECTANGLE_BUTTON, 
      BitmapButton(new BitmapPane(wxBitmap("rectangle.png", wxBITMAP_TYPE_PNG), this, wxID_ANY, wxColour(s_lightBackground), true, wxDefaultPosition, {s_buttonSize, s_buttonSize}), [this]() { OnDrawRectangle(); }) });
    m_buttons.insert({ ButtonType::DRAW_SKIN_TEMPLATE_BUTTON, 
      BitmapButton(new BitmapPane(wxBitmap("skin-template.png", wxBITMAP_TYPE_PNG), this, wxID_ANY, wxColour(s_lightBackground), true, wxDefaultPosition, {s_buttonSize, s_buttonSize}), [this]() { OnDrawSkinTemplate(); }) });
    m_buttons.insert({ ButtonType::ZOOM_IN_BUTTON, 
      BitmapButton(new BitmapPane(wxBitmap("zoom-in.png", wxBITMAP_TYPE_PNG), this, wxID_ANY, wxColour(s_lightBackground), true, wxDefaultPosition, {s_buttonSize, s_buttonSize}), [this]() { 
        ToolboxButtonEvent event(TOOLBOX_BUTTON_PUSHED, GetId());
        event.SetType(ButtonType::ZOOM_IN_BUTTON);
        ProcessWindowEvent(event);
      }) });
    m_buttons.insert({ ButtonType::ZOOM_OUT_BUTTON, 
      BitmapButton(new BitmapPane(wxBitmap("zoom-out.png", wxBITMAP_TYPE_PNG), this, wxID_ANY, wxColour(s_lightBackground), true, wxDefaultPosition, {s_buttonSize, s_buttonSize}), [this]() { 
        ToolboxButtonEvent event(TOOLBOX_BUTTON_PUSHED, GetId());
        event.SetType(ButtonType::ZOOM_OUT_BUTTON);
        ProcessWindowEvent(event);
      }) });

    auto topSizer = new wxBoxSizer(wxVERTICAL);
    auto text = new wxStaticText(this, wxID_ANY, "Toolbox");
    topSizer->Add(text, 0, wxALL, FromDIP(5));

    auto toolboxPaneSizer = new wxWrapSizer(wxHORIZONTAL);
    for (auto& buttonHandler : m_buttons)
    {
      auto button = buttonHandler.second;
      button.pane->Bind(wxEVT_LEFT_DOWN, [this, button, buttonHandler](wxMouseEvent& event)
        {
          if (!button.pane->m_disabled)
          {
            SelectPane(button.pane);
            button.handlerFn();
          }
        });
      toolboxPaneSizer->Add(button.pane, 0, wxALL, FromDIP(5));
    }

    Bind(TOOLBOX_BUTTON_COMMAND_DONE, [this](ToolboxButtonEvent& event)
      {
        auto buttonType = event.GetButtonType();
        auto button = m_buttons.find(buttonType);
        if (button != m_buttons.end())
        {
          button->second.pane->m_selected = false;
          SelectPane(nullptr);
        }
        else
          wxLogError("Toolbox::OnButtonPushed: Button type not found");
      });

    Bind(TOOLBOX_BUTTTON_DISABLED, [this](ToolboButtonStateUpdateEvent& event)
      {
        auto buttonsToDisable = event.GetDisabledButtons();
        auto buttonsToEnable = event.GetEnabledButtons();
        for (auto buttonType : buttonsToDisable)
        {
          auto button = m_buttons.find(buttonType);
          if (button != m_buttons.end())
          {
            button->second.pane->m_selected = false;
            button->second.pane->m_disabled = true;
            button->second.pane->Refresh();
          }
          else
            wxLogError("Toolbox::OnButtonDisabled: Button type not found");
        }

        for (auto buttonType : buttonsToEnable)
        {
          auto button = m_buttons.find(buttonType);
          if (button != m_buttons.end())
          {
            button->second.pane->m_disabled = false;
            button->second.pane->Refresh();
          }
          else
            wxLogError("Toolbox::OnButtonDisabled: Button type not found");
        }
      });

    topSizer->Add(toolboxPaneSizer, 0, wxALL, FromDIP(5));
    SetSizer(topSizer);
  }

  void Toolbox::SelectPane(BitmapPane* pane)
  {
    for (auto buttons : m_buttons)
    {
      buttons.second.pane->m_selected = (buttons.second.pane == pane);
      buttons.second.pane->Refresh();
    }
  }

  void Toolbox::OnScreenshot()
  {
    ToolboxButtonEvent event(TOOLBOX_BUTTON_PUSHED, GetId());
    event.SetType(ButtonType::SCREENSHOT_BUTTON);
    ProcessWindowEvent(event);
  }

  void Toolbox::OnSave()
  {
    ToolboxButtonEvent event(TOOLBOX_BUTTON_PUSHED, GetId());
    event.SetType(ButtonType::SAVE_BUTTON);
    ProcessWindowEvent(event);
  }

  void Toolbox::OnDelete()
  {
    ToolboxButtonEvent event(TOOLBOX_BUTTON_PUSHED, GetId());
    event.SetType(ButtonType::DELETE_BUTTON);
    ProcessWindowEvent(event);
  }

  void Toolbox::OnUndo()
  {
    ToolboxButtonEvent event(TOOLBOX_BUTTON_PUSHED, GetId());
    event.SetType(ButtonType::UNDO_BUTTON);
    ProcessWindowEvent(event);
  }

  void Toolbox::OnCancel()
  {
    ToolboxButtonEvent event(TOOLBOX_BUTTON_PUSHED, GetId());
    event.SetType(ButtonType::CANCEL_BUTTON);
    ProcessWindowEvent(event);
  }

  void Toolbox::OnDrawText()
  {
    ToolboxButtonEvent event(TOOLBOX_BUTTON_PUSHED, GetId());
    event.SetType(ButtonType::DRAW_TEXT_BUTTON);
    ProcessWindowEvent(event);
  }

  void Toolbox::OnDrawIncrementalLetters()
  {
    ToolboxButtonEvent event(TOOLBOX_BUTTON_PUSHED, GetId());
    event.SetType(ButtonType::DRAW_LETTERS_BUTTON);
    ProcessWindowEvent(event);
  }

  void Toolbox::OnDrawArrow()
  {
    ToolboxButtonEvent event(TOOLBOX_BUTTON_PUSHED, GetId());
    event.SetType(ButtonType::DRAW_ARROW_BUTTON);
    ProcessWindowEvent(event);
  }

  void Toolbox::OnDrawCircle()
  {
    ToolboxButtonEvent event(TOOLBOX_BUTTON_PUSHED, GetId());
    event.SetType(ButtonType::DRAW_CIRCLE_BUTTON);
    ProcessWindowEvent(event);
  }

  void Toolbox::OnDrawLine()
  {
    ToolboxButtonEvent event(TOOLBOX_BUTTON_PUSHED, GetId());
    event.SetType(ButtonType::DRAW_LINE_BUTTON);
    ProcessWindowEvent(event);
  }

  void Toolbox::OnDrawMultiline()
  {
    ToolboxButtonEvent event(TOOLBOX_BUTTON_PUSHED, GetId());
    event.SetType(ButtonType::DRAW_MULTILINE_BUTTON);
    ProcessWindowEvent(event);
  }

  void Toolbox::OnDrawRectangle()
  {
    ToolboxButtonEvent event(TOOLBOX_BUTTON_PUSHED, GetId());
    event.SetType(ButtonType::DRAW_RECTANGLE_BUTTON);
    //wxCommandEvent event(TOOLBOX_DRAW_RECTANGLE, GetId());
    //event.SetEventObject(this);
    ProcessWindowEvent(event);
  }

  void Toolbox::OnDrawSkinTemplate()
  {
    ToolboxButtonEvent event(TOOLBOX_BUTTON_PUSHED, GetId());
    event.SetType(ButtonType::DRAW_SKIN_TEMPLATE_BUTTON);
    //wxCommandEvent event(TOOLBOX_DRAW_SKIN_TEMPLATE, GetId());
    //event.SetEventObject(this);
    ProcessWindowEvent(event);
  }

}