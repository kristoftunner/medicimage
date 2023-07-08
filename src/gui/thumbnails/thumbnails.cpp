#include <wx/log.h>
#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "gui/toolbox/attribute_editor_events.h"
#include "thumbnails.h"

namespace app
{

Thumbnails::Thumbnails(wxWindow *parent, wxWindowID, const wxPoint &pos, const wxSize &size)
    : wxScrolled<wxPanel>(parent)
{
    // SetBackgroundColour(wxColour(100, 200, 100));
    m_sizer = new wxBoxSizer(wxVERTICAL);
    m_panelName = new wxStaticText(this, wxID_ANY, "Thumbnails");

    m_sizer->Add(m_panelName, 0, wxALL, FromDIP(5));

    Bind(EVT_PATIENT_SELECTED, &Thumbnails::OnPatientSelected, this);
    Bind(EVT_THUMBNAILS_ADD_PATIENT, &Thumbnails::OnAddPatient, this);
    Bind(EDITOR_SAVE_DOCUMENT, &Thumbnails::OnAddDocument, this);
    Bind(EDITOR_DELETE_DOCUMENT, &Thumbnails::OnDeleteDocument, this);
    Bind(EDITOR_ADD_DOCUMENT, &Thumbnails::OnAddDocument, this);
    Bind(EVT_THUMBNAILS_APP_FOLDER_UPDATE, &Thumbnails::OnUpdateAppFolder, this);
    Bind(wxEVT_SIZE, &Thumbnails::OnResize, this);

    SetSizer(m_sizer);
    SetScrollRate(FromDIP(100), FromDIP(100));
    auto minSize = m_sizer->CalcMin();
    SetVirtualSize(FromDIP(minSize.x), FromDIP(minSize.y));
    UpdatePatientListCtrl();
}

void Thumbnails::SelectPane(BitmapPane *pane)
{
    for (auto colorPane : m_colorPanes)
    {
        colorPane->m_selected = (colorPane == pane);
        colorPane->Refresh();
    }
}

void Thumbnails::OnDeleteDocument(ImageDocumentEvent &event)
{
    m_documentController.OnDeleteDocument(event.GetData());
    UpdateLayout();
}

void Thumbnails::OnAddDocument(ImageDocumentEvent &event)
{
    m_documentController.AddDocument(event.GetData());
    UpdateLayout();
}

void Thumbnails::OnResize(wxSizeEvent &event)
{
    UpdateLayout();
}

void Thumbnails::UpdateLayout()
{
    m_buttonHandlers.clear();
    m_sizer->Clear();
    for (auto pane : m_colorPanes)
        delete pane;
    m_colorPanes.clear();

    m_sizer->Add(m_panelName, 0, wxALL, FromDIP(5));

    auto documents = m_documentController.GetSavedImages();
    if (documents)
    {
        for (auto &document : *(documents.value()))
        {
            m_buttonHandlers.push_back({document.image->GetBitmap(), [this, document]() {
                                            ImageDocumentEvent event(EVT_THUMBNAILS_DOCUMENT_PICK, wxID_ANY);
                                            event.SetData(document);
                                            event.SetEventObject(this);
                                            ProcessWindowEvent(event);
                                        }});
        }

        int panelWidth = static_cast<int>(GetSize().GetWidth() * 0.9);
        for (auto &buttonHandler : m_buttonHandlers)
        {
            auto pane = new BitmapPane(buttonHandler.first, this, wxID_ANY, wxColour(m_lightBackground), false,
                                       wxDefaultPosition, {panelWidth, panelWidth});
            pane->Bind(wxEVT_LEFT_DOWN, [this, pane, buttonHandler](wxMouseEvent &event) {
                SelectPane(pane);
                buttonHandler.second();
            });
            m_colorPanes.push_back(pane);
            m_sizer->Add(pane, 0, wxALL, FromDIP(5));
        }
    }
    auto size = m_sizer->CalcMin();
    SetVirtualSize(FromDIP(size.GetWidth()), FromDIP(size.GetHeight()));
    m_sizer->Layout();
    Refresh();
}

void Thumbnails::OnAddPatient(PatientSelectedEvent &event)
{
    wxLogDebug("Thumbnails::OnAddPatient: %s", event.GetPatientId());
    m_documentController.AddPatient(event.GetPatientId());
    UpdatePatientListCtrl();
    UpdateLayout();
}

void Thumbnails::OnPatientSelected(PatientSelectedEvent &event)
{
    auto selectedPatient = event.GetPatientId();
    m_documentController.SelectPatient(selectedPatient);
    wxLogDebug("Thumbnails::OnPatientSelected: %s", selectedPatient);
    UpdateLayout();
}

void Thumbnails::OnUpdateAppFolder(AppFolderUpdateEvent &event)
{
    m_documentController.UpdateAppFolder(event.GetAppFolder());
    UpdatePatientListCtrl();
    UpdateLayout();
}

void Thumbnails::UpdatePatientListCtrl()
{
    std::vector<UpdatePatientsEvent::Patient> patients;
    auto patientsIds = m_documentController.GetPatientIds();
    for (auto patientId : patientsIds)
    {
        bool selected = patientId == m_documentController.GetSelectedPatientId();
        patients.push_back({patientId, selected});
    }
    UpdatePatientsEvent event(EVT_PATIENT_UPDATED, wxID_ANY);
    event.SetData(patients);
    ProcessWindowEvent(event);
}
} // namespace app