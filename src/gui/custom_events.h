#pragma once

#include <filesystem>
#include <optional>
#include <wx/event.h>

#include "drawing/entity.h"
#include "image_handling/image_saver.h"
#include "toolbox/toolbox_buttons.h"

namespace app
{

using namespace medicimage;

class EntityEvent : public wxCommandEvent
{
  public:
    EntityEvent(wxEventType eventType, wxWindowID id) : wxCommandEvent(eventType, id)
    {
    }

    // Getter and setter for the custom data
    void SetData(const Entity &data)
    {
        m_entity = data;
    }
    std::optional<Entity> GetData()
    {
        return m_entity;
    }

    // Required for event cloning
    wxEvent *Clone() const override
    {
        return new EntityEvent(*this);
    }

  private:
    std::optional<Entity> m_entity;
};

class PatientSelectedEvent : public wxCommandEvent
{
  public:
    PatientSelectedEvent(wxEventType eventType, wxWindowID id) : wxCommandEvent(eventType, id)
    {
    }

    // Getter and setter for the custom data
    void SetData(const std::string &data)
    {
        m_patientId = data;
    }
    const std::string &GetPatientId() const
    {
        return m_patientId;
    }

    // Required for event cloning
    wxEvent *Clone() const override
    {
        return new PatientSelectedEvent(*this);
    }

  private:
    std::string m_patientId = "";
};

class ZoomLevelEvent : public wxCommandEvent
{
  public:
    ZoomLevelEvent(wxEventType eventType, wxWindowID id) : wxCommandEvent(eventType, id)
    {
    }

    // Getter and setter for the custom data
    void SetZoomLevel(const float zoomlevel)
    {
        m_zoomLevel = zoomlevel;
    }
    const float GetZoomlevel() const
    {
        return m_zoomLevel;
    }

    // Required for event cloning
    wxEvent *Clone() const override
    {
        return new ZoomLevelEvent(*this);
    }

  private:
    float m_zoomLevel = 1.0f;
};

class UpdatePatientsEvent : public wxCommandEvent
{
  public:
    struct Patient
    {
        std::string id = "";
        bool selected = false;
    };

    UpdatePatientsEvent(wxEventType eventType, wxWindowID id) : wxCommandEvent(eventType, id)
    {
    }

    // Getter and setter for the custom data
    void SetData(const std::vector<Patient> &data)
    {
        m_patients = data;
    }
    const std::vector<Patient> &GetPatients() const
    {
        return m_patients;
    }

    // Required for event cloning
    wxEvent *Clone() const override
    {
        return new UpdatePatientsEvent(*this);
    }

  private:
    std::vector<Patient> m_patients;
};
class ImageDocumentEvent : public wxCommandEvent
{
  public:
    ImageDocumentEvent(wxEventType eventType, wxWindowID id) : wxCommandEvent(eventType, id)
    {
    }

    // Getter and setter for the custom data
    void SetData(const ImageDocument &data)
    {
        m_data = data;
    }
    ImageDocument &GetData()
    {
        return m_data;
    }

    // Required for event cloning
    wxEvent *Clone() const override
    {
        return new ImageDocumentEvent(*this);
    }

  private:
    ImageDocument m_data;
};

class AppFolderUpdateEvent : public wxCommandEvent
{
  public:
    AppFolderUpdateEvent(wxEventType eventType, wxWindowID id) : wxCommandEvent(eventType, id)
    {
    }

    // Getter and setter for the custom data
    void SetData(const std::string &data)
    {
        m_appFolder = data;
    }
    const std::filesystem::path &GetAppFolder() const
    {
        return m_appFolder;
    }

    // Required for event cloning
    wxEvent *Clone() const override
    {
        return new AppFolderUpdateEvent(*this);
    }

  private:
    std::filesystem::path m_appFolder;
};

class ToolboxButtonEvent : public wxCommandEvent
{
  public:
    ToolboxButtonEvent(wxEventType eventType, wxWindowID id) : wxCommandEvent(eventType, id)
    {
    }

    // Getter and setter for the custom data
    void SetType(const ButtonType button)
    {
        buttonType = button;
    }
    const ButtonType GetButtonType() const
    {
        return buttonType;
    }

    // Required for event cloning
    wxEvent *Clone() const override
    {
        return new ToolboxButtonEvent(*this);
    }

  private:
    ButtonType buttonType = ButtonType::NO_BUTTON;
};

class ToolboButtonStateUpdateEvent : public wxCommandEvent
{
  public:
    ToolboButtonStateUpdateEvent(wxEventType eventType, wxWindowID id) : wxCommandEvent(eventType, id)
    {
    }

    // Getter and setter for the custom data
    void SetDisabledButtons(const std::vector<ButtonType> &buttons)
    {
        buttonsToDisable = buttons;
    }
    void SetEnabledButtons(const std::vector<ButtonType> &buttons)
    {
        buttonsToEnable = buttons;
    }
    const std::vector<ButtonType> &GetDisabledButtons() const
    {
        return buttonsToDisable;
    }
    const std::vector<ButtonType> &GetEnabledButtons() const
    {
        return buttonsToEnable;
    }

    // Required for event cloning
    wxEvent *Clone() const override
    {
        return new ToolboButtonStateUpdateEvent(*this);
    }

  private:
    // TODO: better data structures
    std::vector<ButtonType> buttonsToDisable;
    std::vector<ButtonType> buttonsToEnable;
};

} // namespace app
