#include <wx/stattext.h>
#include <wx/clrpicker.h>
#include <wx/log.h>

#include "attribute_editor.h"
#include "attribute_editor_events.h"
#include "editor/editor_events.h"

namespace app
{

AttributeEditor::AttributeEditor(wxWindow *parent)
  : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL)
{
  auto topSizer = new wxBoxSizer(wxVERTICAL);
  auto panelName = new wxStaticText(this, wxID_ANY, "Properties");
  topSizer->Add(panelName, wxSizerFlags(0).Border(wxALL, FromDIP(5)));
  m_sizer = new wxGridSizer(wxVERTICAL);
  topSizer->Add(m_sizer, wxSizerFlags(1).Expand().Border(wxALL, FromDIP(5)));
  SetSizer(topSizer);
  Bind(EVT_EDITOR_ENTITY_CHANGED, &AttributeEditor::OnEntityAttributeChange, this);
}

void AttributeEditor::OnEntityAttributeChange(EntityEvent &event)
{
  wxLogDebug("AttributeEditor:OnEntityAttributeChange");
  m_sizer->Clear(true);
  auto entity = event.GetData();
  if(entity.HasComponent<ColorComponent>())
  {
    auto& color = entity.GetComponent<ColorComponent>();
    auto colorText = new wxStaticText(this, wxID_ANY, "Color");
    m_sizer->Add(colorText, wxSizerFlags(1).Align(wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL).Border(wxALL, FromDIP(5)));
    auto colorPicker = new wxColourPickerCtrl(this, wxID_ANY, wxColour(color.color.r, color.color.g, color.color.b));
    m_sizer->Add(colorPicker, wxSizerFlags(1).Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL).Border(wxALL, FromDIP(5)));
    colorPicker->Bind(wxEVT_COLOURPICKER_CHANGED, [&](wxColourPickerEvent& event)
    {
      color.color.r = event.GetColour().Red() / 255.0f;
      color.color.g = event.GetColour().Green() / 255.0f;
      color.color.b = event.GetColour().Blue() / 255.0f;

      EntityEvent entityEvent(EVT_ENTITY_ATTRIBUTE_EDITED, wxID_ANY);
      entityEvent.SetData(entity);
      ProcessWindowEvent(entityEvent);
    });

  }
  m_sizer->Layout();
  Refresh();
}

} // namespace app
