#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/log.h>

#include "toolbox.h"

namespace app
{

Toolbox::Toolbox(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size)
  : wxPanel(parent, id, pos, size)
{
  auto sizer = new wxGridSizer(2);
  wxButton *screenshotButton = new wxButton(this, wxID_ANY, wxT("Screenshot"));
  wxButton *saveButton = new wxButton(this, wxID_ANY, wxT("Save"));
  wxButton *deleteButton = new wxButton(this, wxID_ANY, wxT("Delete"));
  wxButton *undoButton = new wxButton(this, wxID_ANY, wxT("Undo"));
  wxButton *drawTextButton = new wxButton(this, wxID_ANY, wxT("Draw Text"));
  wxButton *drawIncrementalLettersButton = new wxButton(this, wxID_ANY, wxT("Draw Incremental Letters"));
  wxButton *drawArrowButton = new wxButton(this, wxID_ANY, wxT("Draw Arrow"));
  wxButton *drawCircleButton = new wxButton(this, wxID_ANY, wxT("Draw Circle"));
  wxButton *drawLineButton = new wxButton(this, wxID_ANY, wxT("Draw Line"));
  wxButton *drawMultilineButton = new wxButton(this, wxID_ANY, wxT("Draw Multiline"));
  wxButton *drawRectangleButton = new wxButton(this, wxID_ANY, wxT("Draw Rectangle"));
  wxButton *drawSkinTemplateButton = new wxButton(this, wxID_ANY, wxT("Draw Skin Template"));

  sizer->Add(screenshotButton, 1, wxEXPAND | wxALL, 5);
  sizer->Add(saveButton, 1, wxEXPAND | wxALL, 5);
  sizer->Add(deleteButton, 1, wxEXPAND | wxALL, 5);
  sizer->Add(undoButton, 1, wxEXPAND | wxALL, 5);
  sizer->Add(drawTextButton, 1, wxEXPAND | wxALL, 5);
  sizer->Add(drawIncrementalLettersButton, 1, wxEXPAND | wxALL, 5);
  sizer->Add(drawArrowButton, 1, wxEXPAND | wxALL, 5);
  sizer->Add(drawCircleButton, 1, wxEXPAND | wxALL, 5);
  sizer->Add(drawLineButton, 0, wxEXPAND | wxALL, 5);
  sizer->Add(drawMultilineButton, 0, wxEXPAND | wxALL, 5);
  sizer->Add(drawRectangleButton, 0, wxEXPAND | wxALL, 5);
  sizer->Add(drawSkinTemplateButton, 0, wxEXPAND | wxALL, 5);
  SetSizer(sizer);

  Bind(wxEVT_BUTTON, &Toolbox::OnScreenshot, this, screenshotButton->GetId());
  Bind(wxEVT_BUTTON, &Toolbox::OnSave, this, saveButton->GetId());
  Bind(wxEVT_BUTTON, &Toolbox::OnDelete, this, deleteButton->GetId());
  Bind(wxEVT_BUTTON, &Toolbox::OnUndo, this, undoButton->GetId());
  Bind(wxEVT_BUTTON, &Toolbox::OnDrawText, this, drawTextButton->GetId());
  Bind(wxEVT_BUTTON, &Toolbox::OnDrawIncrementalLetters, this, drawIncrementalLettersButton->GetId());
  Bind(wxEVT_BUTTON, &Toolbox::OnDrawArrow, this, drawArrowButton->GetId());
  Bind(wxEVT_BUTTON, &Toolbox::OnDrawCircle, this, drawCircleButton->GetId());
  Bind(wxEVT_BUTTON, &Toolbox::OnDrawLine, this, drawLineButton->GetId());
  Bind(wxEVT_BUTTON, &Toolbox::OnDrawMultiline, this, drawMultilineButton->GetId());
  Bind(wxEVT_BUTTON, &Toolbox::OnDrawRectangle, this, drawRectangleButton->GetId());
  Bind(wxEVT_BUTTON, &Toolbox::OnDrawSkinTemplate, this, drawSkinTemplateButton->GetId());

}

void Toolbox::OnScreenshot(wxCommandEvent &event)
{
  wxLogDebug("OnxScreenshot");
}

void Toolbox::OnSave(wxCommandEvent &event)
{
  wxLogDebug("OnSave");
}

void Toolbox::OnDelete(wxCommandEvent &event)
{
  wxLogDebug("OnDelete");
}

void Toolbox::OnUndo(wxCommandEvent &event)
{
  wxLogDebug("OnUndo");
}

void Toolbox::OnDrawText(wxCommandEvent &event)
{
  wxLogDebug("OnDrawText");
}

void Toolbox::OnDrawIncrementalLetters(wxCommandEvent &event)
{
  wxLogDebug("OnDrawIncrementalLetters");
}

void Toolbox::OnDrawArrow(wxCommandEvent &event)
{
  wxLogDebug("OnDrawArrow");
}

void Toolbox::OnDrawCircle(wxCommandEvent &event)
{
  wxLogDebug("OnDrawCircle");
}

void Toolbox::OnDrawLine(wxCommandEvent &event)
{
  wxLogDebug("OnDrawLine");
}

void Toolbox::OnDrawMultiline(wxCommandEvent &event)
{
  wxLogDebug("OnDrawMultiline");
}

void Toolbox::OnDrawRectangle(wxCommandEvent &event)
{
  wxLogDebug("OnDrawRectangle");
}

void Toolbox::OnDrawSkinTemplate(wxCommandEvent &event)
{
  wxLogDebug("OnDrawSkinTemplate");
}

}