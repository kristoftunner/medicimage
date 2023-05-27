#include "canvas.h"
#include <image_handling/image_editor.h>
#include <renderer/texture.h>
#include "toolbox/toolbox.h"
#include "editor.h"

#include <wx/wxprec.h>
#include <wx/dcclient.h>
#include <wx/dcbuffer.h>
#include <wx/wx.h>
#include <wx/image.h>
#include <wx/dcmemory.h>
#include <string>

using namespace medicimage;
namespace app
{
Canvas::Canvas( wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size )
  : wxScrolledWindow( parent, id, pos, size )
{
  m_frameUpdateTimer.SetOwner(this);
  SetBackgroundStyle(wxBG_STYLE_PAINT);
  SetBackgroundColour( *wxWHITE );
  SetCursor(wxCursor(wxCURSOR_ARROW));

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
  m_editor.Init();

  m_dialog = new InfoDialog(this, "Info", m_editor.GetDrawingSheet(), m_editor);
  m_dialog->Show();
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
  if(m_editor.IsDrawingUpdated())
  {
    Refresh();
    m_editor.UpdatedDrawing();
  }
  
  m_dialog->OnUpdate();
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
  }
  
  m_dialog->OnUpdate();
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
  m_editor.OnScreenshot();
  
  m_dialog->OnUpdate();
}

void Canvas::OnSave(wxCommandEvent &event)
{ 
  m_editor.OnSave();
  
  m_dialog->OnUpdate();
}

void Canvas::OnDelete(wxCommandEvent &event)
{
  m_editor.OnDelete();
  
  m_dialog->OnUpdate();
}

void Canvas::OnUndo(wxCommandEvent &event)
{
  m_editor.OnUndo();
  
  m_dialog->OnUpdate();
}

void Canvas::OnDrawText(wxCommandEvent &event)
{
  m_editor.OnDrawText();
  
  m_dialog->OnUpdate();
}

void Canvas::OnDrawIncrementalLetters(wxCommandEvent &event)
{
  m_editor.OnDrawIncrementalLetters();
  
  m_dialog->OnUpdate();
}

void Canvas::OnDrawArrow(wxCommandEvent &event)
{
  m_editor.OnDrawArrow();
  
  m_dialog->OnUpdate();
}

void Canvas::OnDrawCircle(wxCommandEvent &event)
{
  m_editor.OnDrawCircle();
  
  m_dialog->OnUpdate();
}

void Canvas::OnDrawLine(wxCommandEvent &event)
{
  m_editor.OnDrawLine();
  
  m_dialog->OnUpdate();
}

void Canvas::OnDrawMultiline(wxCommandEvent &event)
{
  m_editor.OnDrawMultiline();
  
  m_dialog->OnUpdate();
}

void Canvas::OnDrawRectangle(wxCommandEvent &event)
{
  m_editor.OnDrawRectangle();
  
  m_dialog->OnUpdate();
}

void Canvas::OnDrawSkinTemplate(wxCommandEvent &event)
{ 
  m_editor.OnDrawSkinTemplate();
  
  m_dialog->OnUpdate();
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
}
}
