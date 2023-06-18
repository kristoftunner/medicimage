#pragma once

#include <wx/event.h>

#include "gui/editor/editor_events.h"
#include "gui/custom_events.h"

namespace app
{

wxDECLARE_EVENT(EVT_THUMBNAILS_ADD_PATIENT, PatientSelectedEvent);
wxDECLARE_EVENT(EVT_THUMBNAILS_DOCUMENT_PICK, ImageDocumentEvent);
wxDECLARE_EVENT(EVT_THUMBNAILS_APP_FOLDER_UPDATE, AppFolderUpdateEvent);
  
} // namespace app
