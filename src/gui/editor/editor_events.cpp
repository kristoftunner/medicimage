#include "editor_events.h"

namespace app
{
// TODO REFACTOR: have a common event naming for all user defined events
wxDEFINE_EVENT(EVT_EDITOR_ADD_DOCUMENT, ImageDocumentEvent);
wxDEFINE_EVENT(EVT_EDITOR_SAVE_DOCUMENT, ImageDocumentEvent);
wxDEFINE_EVENT(EVT_EDITOR_DELETE_DOCUMENT, ImageDocumentEvent);
wxDEFINE_EVENT(EVT_EDITOR_ENTITY_CHANGED, EntityEvent);

} // app