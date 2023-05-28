#pragma once

#include <wx/gbsizer.h>
#include <wx/frame.h>
#include <wx/panel.h>
#include <wx/event.h>
#include <wx/listctrl.h>
#include <vector>

#include "editor/editor_events.h"
#include "document_controller.h"
#include "toolbox/bitmappane.h"
#include "thumbnail_events.h"

namespace app
{

using namespace medicimage;
class Thumbnails : public wxScrolled<wxPanel>
{
public:
  Thumbnails( wxWindow *parent, wxWindowID, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize );

private:
  void OnSaveDocument(ImageDocumentEvent& event);
  void OnDeleteDocument(ImageDocumentEvent& event);
  void OnPaint( wxPaintEvent &event );
  void OnAddPatient(PatientEvent& event);
  void OnPatientSelected(wxListEvent& event);

  void UpdatePatientListCtrl();

  void SelectPane(BitmapPane* pane);

private:
  wxBoxSizer* m_sizer;
  wxListCtrl* m_patientList;
  wxStaticText* m_panelName;
  DocumentController m_documentController;
  std::vector<BitmapPane*> m_colorPanes;
  std::vector<BitmapButtonHandler> m_buttonHandlers;
  // TODO: centralize the styling of the app
  const std::string m_lightBackground = "#f4f3f3";
  const std::string m_darkBackground = "#2c2828";
};

}