#pragma once
#include <wx/event.h>

#include "gui/custom_events.h"

namespace app
{
  wxDECLARE_EVENT(TOOLBOX_BUTTON_PUSHED, ToolboxButtonEvent);
  wxDECLARE_EVENT(TOOLBOX_BUTTON_COMMAND_DONE, ToolboxButtonEvent);
  wxDECLARE_EVENT(TOOLBOX_BUTTTON_DISABLED, ToolboButtonStateUpdateEvent);

} // namespace app
