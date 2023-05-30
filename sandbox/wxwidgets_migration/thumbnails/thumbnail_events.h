#pragma once

#include <wx/event.h>

#include "editor/editor_events.h"
#include "custom_events.h"

namespace app
{

wxDECLARE_EVENT(EVT_THUMBNAILS_ADD_PATIENT, PatientEvent);
wxDECLARE_EVENT(EVT_THUMBNAILS_DOCUMENT_PICK, ImageDocumentEvent);
wxDECLARE_EVENT(EVT_THUMBNAILS_APP_FOLDER_UPDATE, AppFolderUpdateEvent);
  
} // namespace app
