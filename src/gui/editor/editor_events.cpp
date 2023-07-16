#include "editor_events.h"

namespace app
{
// TODO REFACTOR: have a common event naming for all user defined events
wxDEFINE_EVENT(EDITOR_ADD_DOCUMENT, ImageDocumentEvent);
wxDEFINE_EVENT(EDITOR_SAVE_DOCUMENT, ImageDocumentEvent);
wxDEFINE_EVENT(EDITOR_DELETE_DOCUMENT, ImageDocumentEvent);
wxDEFINE_EVENT(EDITOR_ENTITY_CHANGED, EntityEvent);
wxDEFINE_EVENT(EDITOR_GET_ACTIVE_PATIENT, EntityEvent);
wxDEFINE_EVENT(EDITOR_ZOOM_LEVEL_CHANGED, ZoomLevelEvent);

} // namespace app