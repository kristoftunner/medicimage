#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "thumbnails.h"

#define POS(r, c)        wxGBPosition(r,c)
#define SPAN(r, c)       wxGBSpan(r,c)
#define TEXTCTRL(text)   new wxTextCtrl(p, wxID_ANY, text)
#define MLTEXTCTRL(text) new wxTextCtrl(p, wxID_ANY, text, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE)

namespace app
{

Thumbnails::Thumbnails( wxWindow *parent, wxWindowID, const wxPoint &pos, const wxSize &size )
  : wxPanel(parent)
{
  SetBackgroundColour(wxColour(100, 200, 100));
}
}