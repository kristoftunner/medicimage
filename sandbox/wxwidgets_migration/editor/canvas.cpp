#include <wx/wxprec.h>
#include <wx/dcclient.h>
#include <wx/dcbuffer.h>
#include <wx/wx.h>
#include <wx/image.h>
#include <wx/dcmemory.h>
#include <wx/textctrl.h>
#include <string>

#include <image_handling/image_editor.h>
#include <renderer/texture.h>
#include "editor.h"
#include "canvas.h"
#include "toolbox/toolbox_events.h"
#include "toolbox/attribute_editor_events.h"
#include "thumbnails/thumbnail_events.h"
#include "editor/editor_events.h"

using namespace medicimage;
namespace app
{
Canvas::Canvas( wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size )
  : wxWindow( parent, id, pos, size )
{
  m_frameUpdateTimer.SetOwner(this);
  SetBackgroundStyle(wxBG_STYLE_PAINT);
  SetBackgroundColour( *wxWHITE );
  SetCursor(wxCursor(wxCURSOR_ARROW));

  m_editor.Init();
  m_dialog = new InfoDialog(this, "Info", m_editor.GetDrawingSheet(), m_editor);
  m_dialog->Show();

  Bind(wxEVT_PAINT, &Canvas::OnPaint, this);
  Bind(wxEVT_LEFT_DOWN, &Canvas::OnMousePressed, this);
  Bind(wxEVT_LEFT_UP, &Canvas::OnMouseReleased, this);
  Bind(wxEVT_MOTION, &Canvas::OnMouseMoved, this);

  Bind(wxEVT_CHAR_HOOK, &Canvas::OnCharInput, this);

  Bind(TOOLBOX_SCREENSHOT, &Canvas::OnScreenshot, this);
  Bind(TOOLBOX_SAVE, &Canvas::OnSave, this);
  Bind(TOOLBOX_DELETE, &Canvas::OnDelete, this);
  Bind(TOOLBOX_UNDO, &Canvas::OnUndo, this);

  Bind(TOOLBOX_DRAW_TEXT, &Canvas::OnDrawText, this);
  Bind(TOOLBOX_DRAW_LETTERS, &Canvas::OnDrawIncrementalLetters, this);
  Bind(TOOLBOX_DRAW_ARROW, &Canvas::OnDrawArrow, this);
  Bind(TOOLBOX_DRAW_CIRCLE, &Canvas::OnDrawCircle, this);
  Bind(TOOLBOX_DRAW_LINE, &Canvas::OnDrawLine, this);
  Bind(TOOLBOX_DRAW_MULTILINE, &Canvas::OnDrawMultiline, this);
  Bind(TOOLBOX_DRAW_RECTANGLE, &Canvas::OnDrawRectangle, this);
  Bind(TOOLBOX_DRAW_SKIN_TEMPLATE, &Canvas::OnDrawSkinTemplate, this);
  Bind(wxEVT_TIMER, &Canvas::OnCameraFrameUpdate, this);  
  
  Bind(EVT_ENTITY_ATTRIBUTE_EDITED, [this](EntityEvent& event)
  {
    Refresh();
  });
  //SetScrollRate(FromDIP(5), FromDIP(5));
  //SetVirtualSize(FromDIP(600), FromDIP(400));

  m_frameUpdateTimer.Start(1000 / 30, wxTIMER_CONTINUOUS);
}

Canvas::~Canvas()
{
}

void Canvas::OnMouseMoved(wxMouseEvent &event)
{
  wxClientDC dc(this);
  PrepareDC(dc);

  const wxPoint pt(event.GetLogicalPosition(dc));
  m_editor.OnMouseMoved({pt.x, pt.y});
  m_dialog->OnUpdate();
  if(m_editor.IsDrawingUpdated())
  {
    m_editor.UpdatedDrawing();
    Refresh();
    UpdateAttributeEditor();
  }
  
}

void Canvas::OnMousePressed(wxMouseEvent &event)
{
  wxClientDC dc(this);
  PrepareDC(dc);

  const wxPoint pt(event.GetLogicalPosition(dc));
  m_editor.OnMousePressed({pt.x, pt.y});
  if(m_editor.IsDrawingUpdated())
  {
    m_editor.UpdatedDrawing();
    Refresh();
    UpdateAttributeEditor();
  }
  
  m_dialog->OnUpdate(); // TODO: check these OnUpdate calls if it is really necessary
  SetFocus();
}

void Canvas::OnMouseReleased(wxMouseEvent &event)
{
  wxClientDC dc(this);
  PrepareDC(dc);

  const wxPoint pt(event.GetLogicalPosition(dc));
  m_editor.OnMouseReleased({pt.x, pt.y});
  if(m_editor.IsDrawingUpdated())
  {
    m_editor.UpdatedDrawing();
    Refresh();
    UpdateAttributeEditor();
  }
  
  m_dialog->OnUpdate();
}

void Canvas::OnCharInput(wxKeyEvent &event)
{
  auto character  = event.GetUnicodeKey();
  std::string str(1, static_cast<char>(character));
  int keycode = event.GetKeyCode();
  if(keycode == WXK_BACK)
    m_editor.OnKeyPressed(Key::MDIK_BACKSPACE);
  else if(keycode == WXK_RETURN || keycode == WXK_NUMPAD_ENTER)
    m_editor.OnKeyPressed(Key::MDIK_RETURN);
  else
    m_editor.OnCharInput(str);

  if(m_editor.IsDrawingUpdated())
  {
    Refresh();
    m_editor.UpdatedDrawing();
    UpdateAttributeEditor();
  }
  
  m_dialog->OnUpdate();
}

void Canvas::OnKeyPressed(wxKeyEvent &event)
{
  int keycode = event.GetKeyCode();
  if(keycode == WXK_BACK)
    m_editor.OnKeyPressed(Key::MDIK_BACKSPACE);
  else if(keycode == WXK_RETURN || keycode == WXK_NUMPAD_ENTER)
    m_editor.OnKeyPressed(Key::MDIK_RETURN);

  if(m_editor.IsDrawingUpdated())
  {
    Refresh();
    m_editor.UpdatedDrawing();
    UpdateAttributeEditor();
  }
  
  m_dialog->OnUpdate();
}

void Canvas::OnPaint(wxPaintEvent &event)
{
  wxAutoBufferedPaintDC dc(this);
  PrepareDC(dc);
  dc.SetBackground(*wxWHITE_BRUSH);
  dc.Clear();
  dc.SetPen(*wxBLACK_PEN);
  dc.SetBrush(*wxTRANSPARENT_BRUSH);

  auto image = m_editor.Draw();
  dc.DrawBitmap(image->GetBitmap(), 0, 0);
  SetVirtualSize(image->GetWidth(), image->GetHeight()); 
}

void Canvas::OnCameraFrameUpdate(wxTimerEvent &event)
{
  if(m_editor.IsCameraFrameUpdated())
  {
    Refresh();
    m_editor.UpdatedFrame();
    m_dialog->OnUpdate();
  }
}

void Canvas::OnScreenshot(wxCommandEvent &event)
{
  auto documentAddEvent = m_editor.OnScreenshot();
  if(documentAddEvent)
  {
    ProcessWindowEvent(documentAddEvent.value());
  }
  m_dialog->OnUpdate();
}

void Canvas::OnSave(wxCommandEvent &event)
{ 
  auto imageSaveEvent = m_editor.OnSave();
  if(imageSaveEvent)
  {
    ProcessWindowEvent(imageSaveEvent.value());
  } 
  m_dialog->OnUpdate();
}

void Canvas::OnDelete(wxCommandEvent &event)
{
  if(m_editor.CanDelete())
  {
    wxMessageDialog dialog(this, "Are you sure you want to delete the selected document?", "Delete document", wxYES_NO | wxICON_QUESTION);
    if(dialog.ShowModal() == wxID_YES)
    {
      auto imageDeleteEvent = m_editor.OnDelete();
      if(imageDeleteEvent)
      {
        ProcessWindowEvent(imageDeleteEvent.value());
      }
      m_dialog->OnUpdate();
      Refresh();
    }
  }
  // TODO: implement this
  
}

void Canvas::OnUndo(wxCommandEvent &event)
{
  if(m_editor.CanUndo())
  {
    wxMessageDialog dialog(this, "Are you sure you want to go back?", "Undo", wxYES_NO | wxICON_QUESTION);
    if(dialog.ShowModal() == wxID_YES)
    {
      m_editor.OnUndo();
      m_dialog->OnUpdate();
      Refresh();
    }
  }
}

void Canvas::OnDocumentPicked(ImageDocumentEvent &event)
{
  m_editor.OnDocumentPicked(event.GetData());
  m_dialog->OnUpdate();
  Refresh();
}

void Canvas::OnDrawText(wxCommandEvent &event)
{
  m_editor.OnDrawText();
  
  UpdateAttributeEditor();
  m_dialog->OnUpdate();
}

void Canvas::OnDrawIncrementalLetters(wxCommandEvent &event)
{
  m_editor.OnDrawIncrementalLetters();
  
  UpdateAttributeEditor();
  m_dialog->OnUpdate();
}

void Canvas::OnDrawArrow(wxCommandEvent &event)
{
  m_editor.OnDrawArrow();
  
  UpdateAttributeEditor();
  m_dialog->OnUpdate();
}

void Canvas::OnDrawCircle(wxCommandEvent &event)
{
  m_editor.OnDrawCircle();
  
  UpdateAttributeEditor();
  m_dialog->OnUpdate();
}

void Canvas::OnDrawLine(wxCommandEvent &event)
{
  m_editor.OnDrawLine();
  
  UpdateAttributeEditor();
  m_dialog->OnUpdate();
}

void Canvas::OnDrawMultiline(wxCommandEvent &event)
{
  m_editor.OnDrawMultiline();
  
  UpdateAttributeEditor();
  m_dialog->OnUpdate();
}

void Canvas::OnDrawRectangle(wxCommandEvent &event)
{
  m_editor.OnDrawRectangle();
  
  UpdateAttributeEditor();
  m_dialog->OnUpdate();
}

void Canvas::OnDrawSkinTemplate(wxCommandEvent &event)
{ 
  m_editor.OnDrawSkinTemplate();
  
  UpdateAttributeEditor();
  m_dialog->OnUpdate();
}

void Canvas::UpdateAttributeEditor()
{
  auto entities = m_editor.GetSelectedEntities();
  if(!entities.empty())
  {
    EntityEvent event(EVT_EDITOR_ENTITY_CHANGED, wxID_ANY);
    event.SetData(entities[0]);
    ProcessWindowEvent(event);
  }
}

InfoDialog::InfoDialog(wxWindow* parent, const wxString& title, DrawingSheet& sheet, Editor& editor)
  : wxFrame(parent, wxID_ANY, title), m_sheet(sheet), m_editor(editor)
{
  // Create dialog content
  m_sizer = new wxBoxSizer(wxVERTICAL);
  auto drawStateText = std::format("DrawState:{}", m_sheet.GetDrawState()->GetName());
  auto drawCommandText = std::format("DrawCommand:{}", m_sheet.GetDrawCommandName());
  auto editorState = std::format("EditorState:{}", m_editor.GetStateName());
  m_drawState = new wxStaticText(this, wxID_ANY, drawStateText);
  m_drawCommand = new wxStaticText(this, wxID_ANY, drawCommandText);
  m_editorState = new wxStaticText(this, wxID_ANY, editorState);
  m_sizer->Add(m_drawState, wxSizerFlags().Align(wxALIGN_TOP).Border(wxALL, FromDIP(1)));  // TODO: properly align this
  m_sizer->Add(m_drawCommand, wxSizerFlags().Align(wxALIGN_TOP).Border(wxALL, FromDIP(1)));
  m_sizer->Add(m_editorState, wxSizerFlags().Align(wxALIGN_TOP).Border(wxALL, FromDIP(1)));
  SetSizerAndFit(m_sizer);
}

void InfoDialog::OnUpdate()
{
  auto drawStateText = std::format("DrawState:{}", m_sheet.GetDrawState()->GetName());
  auto drawCommandText = std::format("DrawCommand:{}", m_sheet.GetDrawCommandName());
  auto editorStateText = std::format("EditorState:{}", m_editor.GetStateName());
  m_drawState->SetLabel(drawStateText);
  m_drawCommand->SetLabel(drawCommandText);
  m_editorState->SetLabel(editorStateText);
  Layout();
}

EditorPanel::EditorPanel(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size)
  : wxPanel(parent, id, pos, size)
{
  auto sizer = new wxBoxSizer(wxVERTICAL);
  m_canvas = new Canvas(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
  
  auto patientIdInput = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
  patientIdInput->Bind(wxEVT_TEXT_ENTER, [this, patientIdInput](wxCommandEvent& event)
  {
    auto id = patientIdInput->GetValue().ToStdString();
    PatientEvent patientEvent(EVT_THUMBNAILS_ADD_PATIENT, wxID_ANY);
    patientEvent.SetData(id);
    ProcessWindowEvent(patientEvent);
    patientIdInput->Clear();
  });

  sizer->Add(m_canvas, wxSizerFlags(1).Expand().Border(wxALL, FromDIP(5)));
  sizer->Add(patientIdInput, wxSizerFlags(0).Expand().Border(wxALL, FromDIP(5)));
  SetSizerAndFit(sizer);

  //relay the events from frame to canvas
  Bind(TOOLBOX_SCREENSHOT, [this](wxCommandEvent& event){this->m_canvas->OnScreenshot(event);});
  Bind(TOOLBOX_SAVE, [this](wxCommandEvent& event){this->m_canvas->OnSave(event);});
  Bind(TOOLBOX_DELETE, [this](wxCommandEvent& event){this->m_canvas->OnDelete(event);});
  Bind(TOOLBOX_UNDO, [this](wxCommandEvent& event){this->m_canvas->OnUndo(event);});
  Bind(TOOLBOX_DRAW_TEXT, [this](wxCommandEvent& event){this->m_canvas->OnDrawText(event);});
  Bind(TOOLBOX_DRAW_LETTERS, [this](wxCommandEvent& event){this->m_canvas->OnDrawIncrementalLetters(event);});
  Bind(TOOLBOX_DRAW_ARROW, [this](wxCommandEvent& event){this->m_canvas->OnDrawArrow(event);});
  Bind(TOOLBOX_DRAW_CIRCLE, [this](wxCommandEvent& event){this->m_canvas->OnDrawCircle(event);});
  Bind(TOOLBOX_DRAW_LINE, [this](wxCommandEvent& event){this->m_canvas->OnDrawLine(event);});
  Bind(TOOLBOX_DRAW_MULTILINE, [this](wxCommandEvent& event){this->m_canvas->OnDrawMultiline(event);});
  Bind(TOOLBOX_DRAW_RECTANGLE, [this](wxCommandEvent& event){this->m_canvas->OnDrawRectangle(event);});
  Bind(TOOLBOX_DRAW_SKIN_TEMPLATE, [this](wxCommandEvent& event){this->m_canvas->OnDrawSkinTemplate(event);});
  Bind(EVT_THUMBNAILS_DOCUMENT_PICK, [this](ImageDocumentEvent& event){this->m_canvas->OnDocumentPicked(event);});

  Bind(EVT_ENTITY_ATTRIBUTE_EDITED, [this](EntityEvent& event){wxPostEvent(this->m_canvas, event);});
}
}
