#include "toolbox_events.h"

namespace app
{
wxDEFINE_EVENT(TOOLBOX_BUTTON_PUSHED, ToolboxButtonEvent);
wxDEFINE_EVENT(TOOLBOX_BUTTON_COMMAND_DONE, ToolboxButtonEvent);
wxDEFINE_EVENT(TOOLBOX_BUTTTON_DISABLED, ToolboButtonStateUpdateEvent);

} // namespace app
