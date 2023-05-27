#pragma once
#include <wx/event.h>

namespace app
{
  wxDECLARE_EVENT(TOOLBOX_SCREENSHOT, wxCommandEvent);
  wxDECLARE_EVENT(TOOLBOX_SAVE, wxCommandEvent);
  wxDECLARE_EVENT(TOOLBOX_DELETE, wxCommandEvent);
  wxDECLARE_EVENT(TOOLBOX_UNDO, wxCommandEvent);

  wxDECLARE_EVENT(TOOLBOX_DRAW_TEXT, wxCommandEvent);
  wxDECLARE_EVENT(TOOLBOX_DRAW_LETTERS, wxCommandEvent);
  wxDECLARE_EVENT(TOOLBOX_DRAW_ARROW, wxCommandEvent);
  wxDECLARE_EVENT(TOOLBOX_DRAW_CIRCLE, wxCommandEvent);
  wxDECLARE_EVENT(TOOLBOX_DRAW_LINE, wxCommandEvent);
  wxDECLARE_EVENT(TOOLBOX_DRAW_MULTILINE, wxCommandEvent);
  wxDECLARE_EVENT(TOOLBOX_DRAW_RECTANGLE, wxCommandEvent);
  wxDECLARE_EVENT(TOOLBOX_DRAW_SKIN_TEMPLATE, wxCommandEvent);

} // namespace app
