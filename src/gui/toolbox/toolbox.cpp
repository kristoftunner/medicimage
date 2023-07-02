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
    : wxScrolled<wxPanel>(parent, id, pos, size)
  {
    m_buttons.insert({ ButtonType::LIVE_CAMERA_BUTTON, 
      BitmapButton(new BitmapPane(wxBitmap("live-camera.png", wxBITMAP_TYPE_PNG), this, wxID_ANY, wxColour(s_lightBackground), true, wxDefaultPosition, {s_buttonSize * 2, s_buttonSize * 2}), [this]() { OnUndo(); }) });
    m_buttons.insert({ ButtonType::SNAPSHOT_BUTTON, 
      BitmapButton(new BitmapPane(wxBitmap("snapshot.png", wxBITMAP_TYPE_PNG), this, wxID_ANY, wxColour(s_lightBackground), true, wxDefaultPosition, {s_buttonSize * 2, s_buttonSize }), [this]() { OnScreenshot(); }) });
    m_buttons.insert({ ButtonType::SAVE_BUTTON,   
      BitmapButton(new BitmapPane(wxBitmap("save.png", wxBITMAP_TYPE_PNG), this, wxID_ANY, wxColour(s_lightBackground), true, wxDefaultPosition, {s_buttonSize, s_buttonSize}), [this]() { OnSave(); }) });
    m_buttons.insert({ ButtonType::DELETE_BUTTON, 
      BitmapButton(new BitmapPane(wxBitmap("delete.png", wxBITMAP_TYPE_PNG), this, wxID_ANY, wxColour(s_lightBackground), true, wxDefaultPosition, {s_buttonSize, s_buttonSize}), [this]() { OnDelete(); }) });
    m_buttons.insert({ ButtonType::SELECT_BUTTON, 
      BitmapButton(new BitmapPane(wxBitmap("select.png", wxBITMAP_TYPE_PNG), this, wxID_ANY, wxColour(s_lightBackground), true, wxDefaultPosition, {s_buttonSize, s_buttonSize}), [this]() { OnCancel(); }) });
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

    auto drawToolSizer = new wxGridSizer(2, 0, 0);
    auto liveCameraSizer = new wxBoxSizer(wxVERTICAL); 
    auto snapshotSizer = new wxBoxSizer(wxVERTICAL);
    auto zoomPanelSizer = new wxBoxSizer(wxHORIZONTAL);

    for (auto& buttonHandler : m_buttons)
    {
      auto& button = buttonHandler.second;
      button.pane->Bind(wxEVT_LEFT_DOWN, [this, button, buttonHandler](wxMouseEvent& event)
        {
          if (!button.pane->m_disabled)
          {
            SelectPane(button.pane);
            button.handlerFn();
          }
        });
      auto buttonType = buttonHandler.first;
      if(buttonType == ButtonType::ZOOM_IN_BUTTON || buttonType == ButtonType::ZOOM_OUT_BUTTON)
          zoomPanelSizer->Add(button.pane, wxSizerFlags(1));
      else if(buttonType == ButtonType::SNAPSHOT_BUTTON)
          snapshotSizer->Add(button.pane, wxSizerFlags(0).Expand());
      else if(buttonType == ButtonType::LIVE_CAMERA_BUTTON)
          liveCameraSizer->Add(button.pane, wxSizerFlags(1));
      else
          drawToolSizer->Add(button.pane, wxSizerFlags(1));
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
        auto& buttonsToDisable = event.GetDisabledButtons();
        auto& buttonsToEnable = event.GetEnabledButtons();
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

    topSizer->Add(liveCameraSizer,  wxSizerFlags(0).Border(wxALL, FromDIP(5)));
    topSizer->Add(snapshotSizer, wxSizerFlags(2).Border(wxALL, FromDIP(5)));
    topSizer->Add(drawToolSizer, wxSizerFlags(0).Border(wxALL, FromDIP(5)));
    topSizer->Add(zoomPanelSizer, wxSizerFlags(0).Border(wxALL, FromDIP(5)));
    SetSizer(topSizer);

    SetScrollRate(FromDIP(100), FromDIP(100));
    auto minSize = topSizer->CalcMin();
    SetVirtualSize(20,100);
    //SetVirtualSize(FromDIP(minSize.x), FromDIP(minSize.y)); 
  }

  void Toolbox::SelectPane(BitmapPane* pane)
  {
    for (auto& buttons : m_buttons)
    {
      buttons.second.pane->m_selected = (buttons.second.pane == pane);
      buttons.second.pane->Refresh();
    }
  }

  void Toolbox::OnScreenshot()
  {
    ToolboxButtonEvent event(TOOLBOX_BUTTON_PUSHED, GetId());
    event.SetType(ButtonType::SNAPSHOT_BUTTON);
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
    event.SetType(ButtonType::LIVE_CAMERA_BUTTON);
    ProcessWindowEvent(event);
  }

  void Toolbox::OnCancel()
  {
    ToolboxButtonEvent event(TOOLBOX_BUTTON_PUSHED, GetId());
    event.SetType(ButtonType::SELECT_BUTTON);
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