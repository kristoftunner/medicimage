#pragma once

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/scrolwin.h>
#include <wx/event.h>
#include <wx/listctrl.h>

#include "drawing/entity.h"
#include "gui/custom_events.h"
#include "gui/thumbnails/document_controller.h"

namespace app
{
using namespace medicimage;



class AttributeEditor : public wxScrolled<wxPanel>
{
public:
  AttributeEditor(wxWindow* parent, DocumentController& controller);
  ~AttributeEditor() = default;
private:
  void OnEntityAttributeChange(EntityEvent& event);
private:
  wxGridSizer* m_gridSizer;
  wxBoxSizer* m_topSizer;
  wxListCtrl* m_patientList;
  DocumentController& m_documentController;
};
  
} // namespace app
