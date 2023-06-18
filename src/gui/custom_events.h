#pragma once

#include <wx/event.h>
#include <filesystem>
#include <optional>

#include "image_handling/image_saver.h"
#include "drawing/entity.h"

namespace app
{

using namespace medicimage;

class EntityEvent : public wxCommandEvent
{
public:
    EntityEvent(wxEventType eventType, wxWindowID id)
        : wxCommandEvent(eventType, id)
    {
    }

    // Getter and setter for the custom data
    void SetData(const Entity& data) { m_entity = data; }
    std::optional<Entity> GetData() { return m_entity; }

    // Required for event cloning
    wxEvent* Clone() const override { return new EntityEvent(*this); }

private:
    std::optional<Entity> m_entity;
};

class PatientSelectedEvent : public wxCommandEvent
{
public:
    PatientSelectedEvent(wxEventType eventType, wxWindowID id)
        : wxCommandEvent(eventType, id)
    {
    }

    // Getter and setter for the custom data
    void SetData(const std::string& data) { m_patientId = data; }
    const std::string& GetPatientId() const { return m_patientId; }

    // Required for event cloning
    wxEvent* Clone() const override { return new PatientSelectedEvent(*this); }

private:
    std::string m_patientId = "";
};

class UpdatePatientsEvent : public wxCommandEvent
{
public:
    struct Patient
    {
      std::string id = "";
      bool selected = false; 
    };

    UpdatePatientsEvent(wxEventType eventType, wxWindowID id)
        : wxCommandEvent(eventType, id)
    {
    }

    // Getter and setter for the custom data
    void SetData(const std::vector<Patient>& data) { m_patients = data; }
    const std::vector<Patient>& GetPatients() const { return m_patients; }

    // Required for event cloning
    wxEvent* Clone() const override { return new UpdatePatientsEvent(*this); }

private:
    std::vector<Patient> m_patients;
};
class ImageDocumentEvent : public wxCommandEvent
{
public:
    ImageDocumentEvent(wxEventType eventType, wxWindowID id)
        : wxCommandEvent(eventType, id)
    {
    }

    // Getter and setter for the custom data
    void SetData(const ImageDocument& data) { m_data = data; }
    ImageDocument& GetData() { return m_data; }

    // Required for event cloning
    wxEvent* Clone() const override { return new ImageDocumentEvent(*this); }

private:
    ImageDocument m_data;
};

class AppFolderUpdateEvent : public wxCommandEvent
{
public:
    AppFolderUpdateEvent(wxEventType eventType, wxWindowID id)
        : wxCommandEvent(eventType, id)
    {
    }

    // Getter and setter for the custom data
    void SetData(const std::string& data) { m_appFolder = data; }
    const std::filesystem::path& GetAppFolder() const { return m_appFolder; }

    // Required for event cloning
    wxEvent* Clone() const override { return new AppFolderUpdateEvent(*this); }

private:
  std::filesystem::path m_appFolder;
};

} // namespace app
