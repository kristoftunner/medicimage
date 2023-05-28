#pragma once

#include <wx/event.h>

#include "editor/editor_events.h"

namespace app
{

class PatientEvent : public wxCommandEvent
{
public:
    PatientEvent(wxEventType eventType, wxWindowID id)
        : wxCommandEvent(eventType, id)
    {
    }

    // Getter and setter for the custom data
    void SetData(const std::string& data) { m_patientId = data; }
    const std::string& GetPatientId() const { return m_patientId; }

    // Required for event cloning
    wxEvent* Clone() const override { return new PatientEvent(*this); }

private:
    std::string m_patientId = "";
};

wxDECLARE_EVENT(EVT_THUMBNAILS_ADD_PATIENT, PatientEvent);
wxDECLARE_EVENT(EVT_THUMBNAILS_DOCUMENT_PICK, ImageDocumentEvent);
  
} // namespace app
