#pragma once

#include <wx/event.h>

#include "custom_events.h"

namespace app
{

using namespace medicimage;

wxDECLARE_EVENT(EVT_ENTITY_ATTRIBUTE_EDITED, EntityEvent);
  
} // namespace app
