#pragma once

#include <wx/event.h>

#include "image_handling/image_saver.h"
namespace app
{

using namespace medicimage;


class ImageDocumentEvent : public wxCommandEvent
{
public:
    ImageDocumentEvent(wxEventType eventType, wxWindowID id)
        : wxCommandEvent(eventType, id)
    {
    }

    // Getter and setter for the custom data
    void SetData(const ImageDocument& data) { m_data = data; }
    ImageDocument GetData() const { return m_data; }

    // Required for event cloning
    wxEvent* Clone() const override { return new ImageDocumentEvent(*this); }

private:
    ImageDocument m_data;
};

wxDECLARE_EVENT(EVT_EDITOR_SAVE_DOCUMENT, ImageDocumentEvent);
wxDECLARE_EVENT(EVT_EDITOR_DELETE_DOCUMENT, ImageDocumentEvent);
  
} // namespace app
