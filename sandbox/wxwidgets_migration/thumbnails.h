#pragma once

#include <wx/gbsizer.h>
#include <wx/frame.h>
#include <wx/panel.h>

namespace app
{

class Thumbnails : public wxPanel
{
public:
  Thumbnails( wxWindow *parent, wxWindowID, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize );

private:
  wxBoxSizer* m_bs;
};

}