#pragma once

#include <wx/event.h>

#include "gui/custom_events.h"
#include "image_handling/image_saver.h"
namespace app
{

using namespace medicimage;

wxDECLARE_EVENT(EVT_EDITOR_ADD_DOCUMENT, ImageDocumentEvent);
wxDECLARE_EVENT(EVT_EDITOR_SAVE_DOCUMENT, ImageDocumentEvent);
wxDECLARE_EVENT(EVT_EDITOR_DELETE_DOCUMENT, ImageDocumentEvent);
wxDECLARE_EVENT(EVT_EDITOR_ENTITY_CHANGED, EntityEvent);
  
} // namespace app
