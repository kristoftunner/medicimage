#pragma once

#include <wx/event.h>

#include "gui/custom_events.h"
#include "image_handling/image_saver.h"
namespace app
{

using namespace medicimage;

wxDECLARE_EVENT(EDITOR_ADD_DOCUMENT, ImageDocumentEvent);
wxDECLARE_EVENT(EDITOR_SAVE_DOCUMENT, ImageDocumentEvent);
wxDECLARE_EVENT(EDITOR_DELETE_DOCUMENT, ImageDocumentEvent);
wxDECLARE_EVENT(EDITOR_ENTITY_CHANGED, EntityEvent);
wxDECLARE_EVENT(EDITOR_GET_ACTIVE_PATIENT, EntityEvent);
  
} // namespace app
