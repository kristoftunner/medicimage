#pragma once

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/scrolwin.h>
#include <wx/event.h>

#include "drawing/entity.h"
#include "custom_events.h"

namespace app
{
using namespace medicimage;



class AttributeEditor : public wxPanel
{
public:
  AttributeEditor(wxWindow* parent);
  ~AttributeEditor() = default;
private:
  void OnEntityAttributeChange(EntityEvent& event);
private:
  wxGridSizer* m_sizer;

};
  
} // namespace app
