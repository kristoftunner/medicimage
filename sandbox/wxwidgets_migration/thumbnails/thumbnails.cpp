#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/panel.h>
#include <wx/log.h>

#include "thumbnails.h"

namespace app
{

Thumbnails::Thumbnails( wxWindow *parent, wxWindowID, const wxPoint &pos, const wxSize &size )
  : wxPanel(parent)
{
  SetBackgroundColour(wxColour(100, 200, 100));
  m_sizer = new wxBoxSizer(wxVERTICAL);
  auto text = new wxStaticText(this, wxID_ANY, "Thumbnails");
  m_sizer->Add(text, 0, wxALL, FromDIP(5));

  Bind(EVT_THUMBNAILS_ADD_PATIENT, &Thumbnails::OnAddPatient, this);
  Bind(EVT_EDITOR_SAVE_DOCUMENT, &Thumbnails::OnSaveDocument, this);
  Bind(EVT_EDITOR_DELETE_DOCUMENT, &Thumbnails::OnDeleteDocument, this);

  SetSizer(m_sizer);
}

void Thumbnails::SelectPane(BitmapPane* pane)
{
  for (auto colorPane : m_colorPanes)
  {
    colorPane->selected = (colorPane == pane);
    colorPane->Refresh();
  }
}

void Thumbnails::OnSaveDocument(ImageDocumentEvent &event)
{
  // TODO: Implement
  wxLogDebug("Thumbnails::OnSaveDocument");
  Refresh();
}

void Thumbnails::OnDeleteDocument(ImageDocumentEvent &event)
{
  // TODO: Implement
  wxLogDebug("Thumbnails::OnDeleteDocument");
  Refresh();
}

void Thumbnails::OnPaint(wxPaintEvent &event)
{
  m_buttonHandlers.clear();
  m_sizer->Clear();
  for(auto pane : m_colorPanes)
    delete pane;
  m_colorPanes.clear();

  auto documents = m_documentController.GetSavedImages();
  if(documents)
  {
    for(auto& document : *(documents.value()))
    {
      m_buttonHandlers.push_back({document.image->GetBitmap(), [this, document]()
      {
        ImageDocumentEvent event(EVT_THUMBNAILS_DOCUMENT_PICK, wxID_ANY);
        event.SetData(document);
        event.SetEventObject(this);
        ProcessWindowEvent(event);
      }});
    }

    for(auto& buttonHandler : m_buttonHandlers)
    {
      auto pane = new BitmapPane(buttonHandler.first, this, wxID_ANY, wxColour(m_lightBackground));
      pane->Bind(wxEVT_LEFT_DOWN, [this, pane, buttonHandler](wxMouseEvent &event)
      {
        SelectPane(pane);
        buttonHandler.second();
      });
      m_colorPanes.push_back(pane);
      m_sizer->Add(pane, 0, wxALL, FromDIP(5));
    }
  }
  
  Layout();

}
void Thumbnails::OnAddPatient(PatientEvent &event)
{
  m_documentController.AddPatient(event.GetPatientId());
}
}