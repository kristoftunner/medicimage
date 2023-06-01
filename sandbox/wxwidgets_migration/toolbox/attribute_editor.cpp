#include <wx/stattext.h>
#include <wx/clrpicker.h>
#include <wx/spinctrl.h>
#include <wx/log.h>

#include "attribute_editor.h"
#include "attribute_editor_events.h"
#include "editor/editor_events.h"
#include "drawing/component_wrappers.h"

namespace app
{

AttributeEditor::AttributeEditor(wxWindow *parent)
  : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL)
{
  m_topSizer = new wxBoxSizer(wxVERTICAL);
  auto panelName = new wxStaticText(this, wxID_ANY, "Properties");
  m_topSizer->Add(panelName, wxSizerFlags(0).Border(wxALL, FromDIP(5)));
  m_gridSizer = new wxGridSizer(2, {5,5});
  m_topSizer->Add(m_gridSizer, wxSizerFlags(1).Expand().Border(wxALL, FromDIP(5)));
  Bind(EVT_EDITOR_ENTITY_CHANGED, &AttributeEditor::OnEntityAttributeChange, this);

  SetSizer(m_topSizer);
}

void AttributeEditor::OnEntityAttributeChange(EntityEvent &event)
{
  wxLogDebug("AttributeEditor:OnEntityAttributeChange");
  m_gridSizer->Clear(true);
  auto entity = event.GetData();
  if(entity.HasComponent<ColorComponent>())
  {
    auto& color = entity.GetComponent<ColorComponent>();
    auto colorText = new wxStaticText(this, wxID_ANY, "Color");
    m_gridSizer->Add(colorText, wxSizerFlags(1).Align(wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL).Border(wxALL, FromDIP(5)));
    auto colorPicker = new wxColourPickerCtrl(this, wxID_ANY, wxColour(color.color.r, color.color.g, color.color.b));
    m_gridSizer->Add(colorPicker, wxSizerFlags(1).Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL).Border(wxALL, FromDIP(5)));
    colorPicker->Bind(wxEVT_COLOURPICKER_CHANGED, [&color, entity, this](wxColourPickerEvent& event)
    {
      color.color.r = event.GetColour().Red() / 255.0f;
      color.color.g = event.GetColour().Green() / 255.0f;
      color.color.b = event.GetColour().Blue() / 255.0f;

      // have to copy because ofsome weird lambda capture error
      auto copyEntity = Entity(entity);
      if(copyEntity.HasComponent<SkinTemplateComponent>())
      {
        SkinTemplateComponentWrapper st(copyEntity);
        st.UpdateShapeAttributes();
      }
      EntityEvent entityEvent(EVT_ENTITY_ATTRIBUTE_EDITED, wxID_ANY);
      entityEvent.SetData(copyEntity);
      ProcessWindowEvent(entityEvent);
    });

  }
  if(entity.HasComponent<ThicknessComponent>())
  {
    auto& thickness = entity.GetComponent<ThicknessComponent>().thickness;
    auto thicknessText = new wxStaticText(this, wxID_ANY, "Thickness");
    m_gridSizer->Add(thicknessText, wxSizerFlags(1).Align(wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL).Border(wxALL, FromDIP(5)));
    // TODO: add min-max values
    auto thicknessPicker = new wxSpinCtrl(this, wxID_ANY, wxString::Format("%d", thickness), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 100);
    m_gridSizer->Add(thicknessPicker, wxSizerFlags(1).Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL).Border(wxALL, FromDIP(5)));
    thicknessPicker->Bind(wxEVT_SPINCTRL, [&thickness, entity, this](wxSpinEvent& event)
    {
      auto copyEntity = Entity(entity);
      thickness = event.GetValue(); 
      if(copyEntity.HasComponent<SkinTemplateComponent>())
      {
        SkinTemplateComponentWrapper st(copyEntity);
        st.UpdateShapeAttributes();
      }

      EntityEvent entityEvent(EVT_ENTITY_ATTRIBUTE_EDITED, wxID_ANY);
      entityEvent.SetData(copyEntity);
      ProcessWindowEvent(entityEvent);
    });
  }
  
  if(entity.HasComponent<TextComponent>())
  {
    auto& fontSize = entity.GetComponent<TextComponent>().fontSize;  
    auto fontSizeText = new wxStaticText(this, wxID_ANY, "Font Size");
    m_gridSizer->Add(fontSizeText, wxSizerFlags(1).Align(wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL).Border(wxALL, FromDIP(5)));
    // TODO: add min-max values
    auto fontSizePicker = new wxSpinCtrl(this, wxID_ANY, wxString::Format("%d", fontSize), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 100);
    m_gridSizer->Add(fontSizePicker, wxSizerFlags(1).Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL).Border(wxALL, FromDIP(5)));
    fontSizePicker->Bind(wxEVT_SPINCTRL, [&fontSize, entity, this](wxSpinEvent& event)
    {
      auto copyEntity = Entity(entity);
      fontSize = event.GetValue();
      if(copyEntity.HasComponent<SkinTemplateComponent>())
      {
        SkinTemplateComponentWrapper st(copyEntity);
        st.UpdateShapeAttributes();
      }

      EntityEvent entityEvent(EVT_ENTITY_ATTRIBUTE_EDITED, wxID_ANY);
      entityEvent.SetData(copyEntity);
      ProcessWindowEvent(entityEvent);
    });
  }
  
  if(entity.HasComponent<SkinTemplateComponent>())
  {
    SkinTemplateComponentWrapper st(entity);
    auto verticalSliceCountBounds = st.GetVerticalSliceCountBounds();
    auto leftHorSliceCountBounds = st.GetLeftHorizontalSliceCountBounds();
    auto rightHorSliceCountBounds = st.GetRightHorizontalSliceCountBounds();

    auto& vertSliceCount = entity.GetComponent<SkinTemplateComponent>().vertSliceCount;
    auto& leftHorSliceCount = entity.GetComponent<SkinTemplateComponent>().leftHorSliceCount;
    auto& rightHorSliceCount = entity.GetComponent<SkinTemplateComponent>().rightHorSliceCount;
    
    auto verticalSliceCountText = new wxStaticText(this, wxID_ANY, "Vertical Slice Count");
    m_gridSizer->Add(verticalSliceCountText, wxSizerFlags(1).Align(wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL).Border(wxALL, FromDIP(5)));
    auto verticalSliceCountPicker = new wxSpinCtrl(this, wxID_ANY, wxString::Format("%d", vertSliceCount), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, verticalSliceCountBounds.x, verticalSliceCountBounds.y);
    m_gridSizer->Add(verticalSliceCountPicker, wxSizerFlags(1).Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL).Border(wxALL, FromDIP(5)));
    verticalSliceCountPicker->Bind(wxEVT_SPINCTRL, [&vertSliceCount, entity, this](wxSpinEvent& event)
    {
      vertSliceCount = event.GetValue();
      SkinTemplateComponentWrapper st(entity);
      st.UpdateShapeAttributes();

      EntityEvent entityEvent(EVT_ENTITY_ATTRIBUTE_EDITED, wxID_ANY);
      entityEvent.SetData(entity);
      ProcessWindowEvent(entityEvent);
    });

    auto leftHorSliceCountText = new wxStaticText(this, wxID_ANY, "Left Slice Count");
    m_gridSizer->Add(leftHorSliceCountText, wxSizerFlags(1).Align(wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL).Border(wxALL, FromDIP(5)));
    auto leftHorSliceCountPicker = new wxSpinCtrl(this, wxID_ANY, wxString::Format("%d", leftHorSliceCount), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, leftHorSliceCountBounds.x, leftHorSliceCountBounds.y);
    m_gridSizer->Add(leftHorSliceCountPicker, wxSizerFlags(1).Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL).Border(wxALL, FromDIP(5)));
    leftHorSliceCountPicker->Bind(wxEVT_SPINCTRL, [&leftHorSliceCount, entity, this](wxSpinEvent& event)
    {
      leftHorSliceCount = event.GetValue();
      SkinTemplateComponentWrapper st(entity);
      st.UpdateShapeAttributes();

      EntityEvent entityEvent(EVT_ENTITY_ATTRIBUTE_EDITED, wxID_ANY);
      entityEvent.SetData(entity);
      this->ProcessWindowEvent(entityEvent);
    });

    auto rightHorSliceCountText = new wxStaticText(this, wxID_ANY, "Right Slice Count");
    m_gridSizer->Add(rightHorSliceCountText, wxSizerFlags(1).Align(wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL).Border(wxALL, FromDIP(5)));
    auto rightHorSliceCountPicker = new wxSpinCtrl(this, wxID_ANY, wxString::Format("%d", rightHorSliceCount), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, rightHorSliceCountBounds.x, rightHorSliceCountBounds.y);
    m_gridSizer->Add(rightHorSliceCountPicker, wxSizerFlags(1).Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL).Border(wxALL, FromDIP(5)));
    rightHorSliceCountPicker->Bind(wxEVT_SPINCTRL, [&rightHorSliceCount, entity, this](wxSpinEvent& event)
    {
      rightHorSliceCount = event.GetValue();
      SkinTemplateComponentWrapper st(entity);
      st.UpdateShapeAttributes();

      EntityEvent entityEvent(EVT_ENTITY_ATTRIBUTE_EDITED, wxID_ANY);
      entityEvent.SetData(entity);
      ProcessWindowEvent(entityEvent);
    });
  }

  auto size = m_topSizer->CalcMin();
  m_gridSizer->Layout();
  m_topSizer->Layout();
  Refresh();
}

} // namespace app
