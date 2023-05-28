#include "toolbox_events.h"

namespace app
{

wxDEFINE_EVENT(TOOLBOX_SCREENSHOT, wxCommandEvent);
wxDEFINE_EVENT(TOOLBOX_SAVE, wxCommandEvent);
wxDEFINE_EVENT(TOOLBOX_DELETE, wxCommandEvent);
wxDEFINE_EVENT(TOOLBOX_UNDO, wxCommandEvent);

wxDEFINE_EVENT(TOOLBOX_DRAW_TEXT, wxCommandEvent);
wxDEFINE_EVENT(TOOLBOX_DRAW_LETTERS, wxCommandEvent);
wxDEFINE_EVENT(TOOLBOX_DRAW_ARROW, wxCommandEvent);
wxDEFINE_EVENT(TOOLBOX_DRAW_CIRCLE, wxCommandEvent);
wxDEFINE_EVENT(TOOLBOX_DRAW_LINE, wxCommandEvent);
wxDEFINE_EVENT(TOOLBOX_DRAW_MULTILINE, wxCommandEvent);
wxDEFINE_EVENT(TOOLBOX_DRAW_RECTANGLE, wxCommandEvent);
wxDEFINE_EVENT(TOOLBOX_DRAW_SKIN_TEMPLATE, wxCommandEvent);
  
} // namespace app
