#include <wx/clrpicker.h>
#include <wx/dcbuffer.h>
#include <wx/log.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>

#include "attribute_editor.h"
#include "attribute_editor_events.h"
#include "drawing/component_wrappers.h"
#include "gui/editor/editor_events.h"

namespace app
{

AttributeEditor::AttributeEditor(wxWindow *parent, DocumentController &controller)
    : wxScrolled<wxPanel>(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL),
      m_documentController(controller)
{
    // SetBackgroundStyle(wxBG_STYLE_PAINT);
    m_topSizer = new wxBoxSizer(wxVERTICAL);
    auto panelName = new wxStaticText(this, wxID_ANY, "Properties");
    m_patientList = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
    m_patientList->InsertColumn(0, "Patient IDs");
    m_gridSizer = new wxGridSizer(2, {0, 0});
    m_topSizer->Add(panelName, wxSizerFlags(0).Border(wxALL, FromDIP(5)));
    m_topSizer->Add(m_gridSizer, wxSizerFlags(0).Border(wxALL, FromDIP(5)).Align(wxALIGN_TOP));
    m_topSizer->Add(m_patientList, wxSizerFlags(0).Expand().Border(wxALL, FromDIP(5)));

    std::vector<UpdatePatientsEvent::Patient> patients;
    auto patientsIds = m_documentController.GetPatientIds();
    int index = 0;
    for (auto id : patientsIds)
    {
        this->m_patientList->InsertItem(index, id);
        bool selected = id == m_documentController.GetSelectedPatientId();
        if (selected)
        {
            this->m_patientList->SetItemState(index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        }
        index++;
    }

    m_patientList->Bind(wxEVT_LIST_ITEM_SELECTED, [this](wxListEvent &event) {
        auto selectedIndex = event.GetIndex();
        std::string selectedItem = this->m_patientList->GetItemText(selectedIndex).ToStdString();
        auto patientEvent = PatientSelectedEvent(EVT_PATIENT_SELECTED, wxID_ANY);
        patientEvent.SetData(selectedItem);
        wxLogDebug("Thumbnails::OnPatientSelected: %s", selectedItem);
        ProcessWindowEvent(patientEvent);
    });
    Bind(EVT_EDITOR_ENTITY_CHANGED, &AttributeEditor::OnEntityAttributeChange, this);
    Bind(EVT_PATIENT_UPDATED, [this](UpdatePatientsEvent &event) {
        this->m_patientList->DeleteAllItems();
        auto patients = event.GetPatients();
        int index = 0;
        for (auto patient : patients)
        {
            this->m_patientList->InsertItem(index, patient.id);
            if (patient.selected)
            {
                this->m_patientList->SetItemState(index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            }
            index++;
        }

        wxLogDebug("AttributeEditor::EVT_PATIENT_UPDATED");
    });
    // Bind(wxEVT_PAINT, &AttributeEditor::Draw, this);

    SetSizer(m_topSizer);
    SetScrollRate(FromDIP(50), FromDIP(50));
    auto minSize = m_topSizer->CalcMin();
    SetVirtualSize(FromDIP(minSize.x), FromDIP(minSize.y));
}

void AttributeEditor::Draw(wxPaintEvent &event)
{
}

void AttributeEditor::OnEntityAttributeChange(EntityEvent &event)
{
    wxLogDebug("AttributeEditor:OnEntityAttributeChange");
    m_gridSizer->Clear(true);
    auto optionalEntity = event.GetData();
    if (optionalEntity.has_value())
    {
        auto entity = optionalEntity.value();
        if (entity.HasComponent<ColorComponent>())
        {
            auto &color = entity.GetComponent<ColorComponent>();
            auto colorText = new wxStaticText(this, wxID_ANY, "Color");
            m_gridSizer->Add(colorText, wxSizerFlags(1).Align(wxALIGN_RIGHT | wxALIGN_TOP).Border(wxALL, FromDIP(2)));
            auto colorPicker =
                new wxColourPickerCtrl(this, wxID_ANY, wxColour(color.color.r, color.color.g, color.color.b));
            m_gridSizer->Add(colorPicker, wxSizerFlags(1).Align(wxALIGN_LEFT | wxALIGN_TOP).Border(wxALL, FromDIP(2)));
            colorPicker->Bind(wxEVT_COLOURPICKER_CHANGED, [&color, entity, this](wxColourPickerEvent &event) {
                color.color.r = event.GetColour().Red() / 255.0f;
                color.color.g = event.GetColour().Green() / 255.0f;
                color.color.b = event.GetColour().Blue() / 255.0f;

                // have to copy because ofsome weird lambda capture error
                auto copyEntity = Entity(entity);
                if (copyEntity.HasComponent<SkinTemplateComponent>())
                {
                    SkinTemplateComponentWrapper st(copyEntity);
                    st.UpdateShapeAttributes();
                }
                EntityEvent entityEvent(EVT_ENTITY_ATTRIBUTE_EDITED, wxID_ANY);
                entityEvent.SetData(copyEntity);
                ProcessWindowEvent(entityEvent);
            });
        }
        if (entity.HasComponent<ThicknessComponent>())
        {
            auto &thickness = entity.GetComponent<ThicknessComponent>().thickness;
            auto thicknessText = new wxStaticText(this, wxID_ANY, "Thickness");
            m_gridSizer->Add(thicknessText,
                             wxSizerFlags(0).Align(wxALIGN_RIGHT | wxALIGN_TOP).Border(wxALL, FromDIP(2)));
            // TODO: add min-max values
            auto thicknessPicker = new wxSpinCtrl(this, wxID_ANY, wxString::Format("%d", thickness), wxDefaultPosition,
                                                  wxDefaultSize, wxSP_ARROW_KEYS, 1, 100);
            m_gridSizer->Add(thicknessPicker,
                             wxSizerFlags(1).Align(wxALIGN_LEFT | wxALIGN_TOP).Border(wxALL, FromDIP(2)));
            thicknessPicker->Bind(wxEVT_SPINCTRL, [&thickness, entity, this](wxSpinEvent &event) {
                auto copyEntity = Entity(entity);
                thickness = event.GetValue();
                if (copyEntity.HasComponent<SkinTemplateComponent>())
                {
                    SkinTemplateComponentWrapper st(copyEntity);
                    st.UpdateShapeAttributes();
                }

                EntityEvent entityEvent(EVT_ENTITY_ATTRIBUTE_EDITED, wxID_ANY);
                entityEvent.SetData(copyEntity);
                ProcessWindowEvent(entityEvent);
            });
        }

        if (entity.HasComponent<TextComponent>())
        {
            auto &fontSize = entity.GetComponent<TextComponent>().fontSize;
            auto fontSizeText = new wxStaticText(this, wxID_ANY, "Font size");
            m_gridSizer->Add(fontSizeText,
                             wxSizerFlags(0).Align(wxALIGN_RIGHT | wxALIGN_TOP).Border(wxALL, FromDIP(2)));
            // TODO: add min-max values
            auto fontSizePicker = new wxSpinCtrl(this, wxID_ANY, wxString::Format("%d", fontSize), wxDefaultPosition,
                                                 wxDefaultSize, wxSP_ARROW_KEYS, 1, 100);
            m_gridSizer->Add(fontSizePicker,
                             wxSizerFlags(1).Align(wxALIGN_LEFT | wxALIGN_TOP).Border(wxALL, FromDIP(2)));
            fontSizePicker->Bind(wxEVT_SPINCTRL, [&fontSize, entity, this](wxSpinEvent &event) {
                auto copyEntity = Entity(entity);
                fontSize = event.GetValue();
                if (copyEntity.HasComponent<SkinTemplateComponent>())
                {
                    SkinTemplateComponentWrapper st(copyEntity);
                    st.UpdateShapeAttributes();
                }

                EntityEvent entityEvent(EVT_ENTITY_ATTRIBUTE_EDITED, wxID_ANY);
                entityEvent.SetData(copyEntity);
                ProcessWindowEvent(entityEvent);
            });
        }

        if (entity.HasComponent<SkinTemplateComponent>())
        {
            SkinTemplateComponentWrapper st(entity);
            auto verticalSliceCountBounds = st.GetVerticalSliceCountBounds();
            auto leftHorSliceCountBounds = st.GetLeftHorizontalSliceCountBounds();
            auto rightHorSliceCountBounds = st.GetRightHorizontalSliceCountBounds();

            auto &vertSliceCount = entity.GetComponent<SkinTemplateComponent>().vertSliceCount;
            auto &leftHorSliceCount = entity.GetComponent<SkinTemplateComponent>().leftHorSliceCount;
            auto &rightHorSliceCount = entity.GetComponent<SkinTemplateComponent>().rightHorSliceCount;

            auto verticalSliceCountText = new wxStaticText(this, wxID_ANY, "Vertical slices");
            m_gridSizer->Add(verticalSliceCountText,
                             wxSizerFlags(0).Align(wxALIGN_RIGHT | wxALIGN_TOP).Border(wxALL, FromDIP(2)));
            auto verticalSliceCountPicker =
                new wxSpinCtrl(this, wxID_ANY, wxString::Format("%d", vertSliceCount), wxDefaultPosition, wxDefaultSize,
                               wxSP_ARROW_KEYS, verticalSliceCountBounds.x, verticalSliceCountBounds.y);
            m_gridSizer->Add(verticalSliceCountPicker,
                             wxSizerFlags(1).Align(wxALIGN_LEFT | wxALIGN_TOP).Border(wxALL, FromDIP(2)));
            verticalSliceCountPicker->Bind(wxEVT_SPINCTRL, [&vertSliceCount, entity, this](wxSpinEvent &event) {
                vertSliceCount = event.GetValue();
                SkinTemplateComponentWrapper st(entity);
                st.UpdateShapeAttributes();

                EntityEvent entityEvent(EVT_ENTITY_ATTRIBUTE_EDITED, wxID_ANY);
                entityEvent.SetData(entity);
                ProcessWindowEvent(entityEvent);
            });

            auto leftHorSliceCountText = new wxStaticText(this, wxID_ANY, "Left slices");
            m_gridSizer->Add(leftHorSliceCountText,
                             wxSizerFlags(0).Align(wxALIGN_RIGHT | wxALIGN_TOP).Border(wxALL, FromDIP(2)));
            auto leftHorSliceCountPicker =
                new wxSpinCtrl(this, wxID_ANY, wxString::Format("%d", leftHorSliceCount), wxDefaultPosition,
                               wxDefaultSize, wxSP_ARROW_KEYS, leftHorSliceCountBounds.x, leftHorSliceCountBounds.y);
            m_gridSizer->Add(leftHorSliceCountPicker,
                             wxSizerFlags(1).Align(wxALIGN_LEFT | wxALIGN_TOP).Border(wxALL, FromDIP(2)));
            leftHorSliceCountPicker->Bind(wxEVT_SPINCTRL, [&leftHorSliceCount, entity, this](wxSpinEvent &event) {
                leftHorSliceCount = event.GetValue();
                SkinTemplateComponentWrapper st(entity);
                st.UpdateShapeAttributes();

                EntityEvent entityEvent(EVT_ENTITY_ATTRIBUTE_EDITED, wxID_ANY);
                entityEvent.SetData(entity);
                this->ProcessWindowEvent(entityEvent);
            });

            auto rightHorSliceCountText = new wxStaticText(this, wxID_ANY, "Right slices");
            m_gridSizer->Add(rightHorSliceCountText,
                             wxSizerFlags(0).Align(wxALIGN_RIGHT | wxALIGN_TOP).Border(wxALL, FromDIP(2)));
            auto rightHorSliceCountPicker =
                new wxSpinCtrl(this, wxID_ANY, wxString::Format("%d", rightHorSliceCount), wxDefaultPosition,
                               wxDefaultSize, wxSP_ARROW_KEYS, rightHorSliceCountBounds.x, rightHorSliceCountBounds.y);
            m_gridSizer->Add(rightHorSliceCountPicker,
                             wxSizerFlags(1).Align(wxALIGN_LEFT | wxALIGN_TOP).Border(wxALL, FromDIP(2)));
            rightHorSliceCountPicker->Bind(wxEVT_SPINCTRL, [&rightHorSliceCount, entity, this](wxSpinEvent &event) {
                rightHorSliceCount = event.GetValue();
                SkinTemplateComponentWrapper st(entity);
                st.UpdateShapeAttributes();

                EntityEvent entityEvent(EVT_ENTITY_ATTRIBUTE_EDITED, wxID_ANY);
                entityEvent.SetData(entity);
                ProcessWindowEvent(entityEvent);
            });
        }

        auto minSize = m_topSizer->CalcMin();
        SetVirtualSize(FromDIP(minSize.x), FromDIP(minSize.y));
        m_gridSizer->Layout();
        Refresh();
    }
}

} // namespace app
