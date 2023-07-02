#include "thumbnail_events.h"

namespace app
{

wxDEFINE_EVENT(EVT_THUMBNAILS_ADD_PATIENT, PatientSelectedEvent);
wxDEFINE_EVENT(EVT_THUMBNAILS_DOCUMENT_PICK, ImageDocumentEvent);
wxDEFINE_EVENT(EVT_THUMBNAILS_APP_FOLDER_UPDATE, AppFolderUpdateEvent);

} // namespace app
