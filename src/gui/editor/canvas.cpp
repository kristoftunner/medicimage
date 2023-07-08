#include <string>
#include <wx/dcbuffer.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/graphics.h>
#include <wx/image.h>
#include <wx/textctrl.h>
#include <wx/wx.h>
#include <wx/wxprec.h>

#include "canvas.h"
#include "editor.h"
#include "gui/editor/editor_events.h"
#include "gui/thumbnails/thumbnail_events.h"
#include "gui/toolbox/attribute_editor_events.h"
#include "gui/toolbox/toolbox_events.h"
#include <image_handling/image_editor.h>
#include <renderer/texture.h>
#include "gui/toolbox/bitmappane.h"

using namespace medicimage;
namespace app
{
Canvas::Canvas(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size)
    : wxWindow(parent, id, pos, size)
{
    m_frameUpdateTimer.SetOwner(this);
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetBackgroundColour({128, 128, 128});
    SetCursor(wxCursor(wxCURSOR_ARROW));

    m_editor.Init();
    m_dialog = new InfoDialog(this, "Info", m_editor.GetDrawingSheet(), m_editor);
    m_dialog->Show(false);

    Bind(wxEVT_PAINT, &Canvas::OnPaint, this);
    Bind(wxEVT_LEFT_DOWN, &Canvas::OnMousePressed, this);
    Bind(wxEVT_LEFT_UP, &Canvas::OnMouseReleased, this);
    Bind(wxEVT_MOTION, &Canvas::OnMouseMoved, this);

    Bind(wxEVT_CHAR_HOOK, &Canvas::OnCharInput, this);

    Bind(wxEVT_TIMER, &Canvas::OnCameraFrameUpdate, this);

    Bind(EVT_ENTITY_ATTRIBUTE_EDITED, [this](EntityEvent &event) { Refresh(); });

    m_frameUpdateTimer.Start(1000 / 30, wxTIMER_CONTINUOUS);
}

Canvas::~Canvas()
{
}

// clamps the mouse position only onto the image, not to the background
glm::vec2 Canvas::CalcCorrectedMousePos(glm::vec2 pos)
{
    auto maxSize = glm::vec2(m_canvasSize.x, m_canvasSize.y) - m_canvasBorder * 2.0f;
    glm::vec2 correctedPos = clamp(pos - m_canvasBorder, glm::vec2(0.0f), maxSize);
    return correctedPos / m_canvasScale;
}

void Canvas::OnMouseMoved(wxMouseEvent &event)
{
    // BIG TODO: refactor this button disabling stuff
    wxClientDC dc(this);
    m_mousePoint = wxPoint(event.GetLogicalPosition(dc));
    auto eventFromEditor = m_editor.OnMouseMoved(CalcCorrectedMousePos({m_mousePoint.x, m_mousePoint.y}));
    if (eventFromEditor)
        ProcessWindowEvent(eventFromEditor.value());

    m_dialog->OnUpdate();
    if (m_editor.IsDrawingUpdated())
    {
        m_editor.UpdatedDrawing();
        Refresh();
        UpdateAttributeEditor();
    }
}

void Canvas::OnMousePressed(wxMouseEvent &event)
{
    wxClientDC dc(this);
    m_mousePoint = wxPoint(event.GetLogicalPosition(dc));
    auto eventFromEditor = m_editor.OnMousePressed(CalcCorrectedMousePos({m_mousePoint.x, m_mousePoint.y}));
    if (eventFromEditor)
        ProcessWindowEvent(eventFromEditor.value());

    if (m_editor.IsDrawingUpdated())
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
    m_mousePoint = wxPoint(event.GetLogicalPosition(dc));

    auto eventFromEditor = m_editor.OnMouseReleased(CalcCorrectedMousePos({m_mousePoint.x, m_mousePoint.y}));
    if (eventFromEditor)
        ProcessWindowEvent(eventFromEditor.value());

    if (m_editor.IsDrawingUpdated())
    {
        m_editor.UpdatedDrawing();
        Refresh();
        UpdateAttributeEditor();
    }

    m_dialog->OnUpdate();
}

void Canvas::OnCharInput(wxKeyEvent &event)
{
    auto character = event.GetUnicodeKey();
    std::string str(1, static_cast<char>(character));
    int keycode = event.GetKeyCode();
    if (keycode == WXK_BACK)
    {
        auto eventFromEditor = m_editor.OnKeyPressed(Key::MDIK_BACKSPACE);
        if (eventFromEditor)
            ProcessWindowEvent(eventFromEditor.value());
    }
    else if (keycode == WXK_RETURN || keycode == WXK_NUMPAD_ENTER)
    {
        auto eventFromEditor = m_editor.OnKeyPressed(Key::MDIK_RETURN);
        if (eventFromEditor)
            ProcessWindowEvent(eventFromEditor.value());
    }
    else if (keycode == WXK_DELETE)
    {
        auto eventFromEditor = m_editor.OnKeyPressed(Key::MDIK_DELETE);
        if (eventFromEditor)
            ProcessWindowEvent(eventFromEditor.value());
    }
    else
    {
        auto eventFromEditor = m_editor.OnCharInput(str);
        if (eventFromEditor)
            ProcessWindowEvent(eventFromEditor.value());
    }

    if (m_editor.IsDrawingUpdated())
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
    if (keycode == WXK_BACK)
    {
        auto eventFromEditor = m_editor.OnKeyPressed(Key::MDIK_BACKSPACE);
        if (eventFromEditor)
            ProcessWindowEvent(eventFromEditor.value());
    }
    else if (keycode == WXK_RETURN || keycode == WXK_NUMPAD_ENTER)
    {
        auto eventFromEditor = m_editor.OnKeyPressed(Key::MDIK_RETURN);
        if (eventFromEditor)
            ProcessWindowEvent(eventFromEditor.value());
    }

    if (m_editor.IsDrawingUpdated())
    {
        Refresh();
        m_editor.UpdatedDrawing();
        UpdateAttributeEditor();
    }

    m_dialog->OnUpdate();
}

std::unique_ptr<Image2D> Canvas::DummyDraw()
{
    auto image = std::make_unique<Image2D>("Checkerboard.png");

    wxBitmap::Rescale(image->GetBitmap(), {1280, 780});
    wxMemoryDC memDC(image->GetBitmap());
    auto gc = wxGraphicsContext::Create(memDC);
    if (gc)
    {

        gc->SetPen(*wxWHITE_PEN);
        gc->SetBrush(*wxWHITE_BRUSH);
        gc->DrawRectangle(0, 0, 1280, 780);
        auto width = m_secondPoint.x - m_firstPoint.x;
        auto height = m_secondPoint.y - m_firstPoint.y;
        gc->SetPen(*wxRED_PEN);
        for (int i = 0; i < 10; ++i)
        {
            gc->DrawRectangle(m_firstPoint.x + i * 20, m_firstPoint.y + i * 20, width, height);
        }
        delete gc;
    }
    memDC.SelectObject(wxNullBitmap);

    return std::move(image);
}
void Canvas::Draw(wxDC &dc)
{
    dc.SetBackground(*wxGREY_BRUSH);
    dc.Clear();
    auto image = m_editor.Draw();
    // calculate new size of the image, so that it will fit into the window
    auto size = GetSize();
    auto imageSize = glm::vec2{image->GetWidth(), image->GetHeight()};
    m_canvasScale = std::min((float)size.x / image->GetWidth(), (float)size.y / image->GetHeight());
    auto newWidth = image->GetWidth() * m_canvasScale;
    auto newHeight = image->GetHeight() * m_canvasScale;
    m_canvasBorder = (glm::vec2{size.x, size.y} - glm::vec2(newWidth, newHeight)) / 2.0f;

    dc.SetUserScale(m_canvasScale, m_canvasScale);
    dc.SetDeviceOrigin(static_cast<int>(m_canvasBorder.x), static_cast<int>(m_canvasBorder.y));
    dc.DrawBitmap(image->GetBitmap(), 0, 0);
}

void Canvas::OnPaint(wxPaintEvent &event)
{
    wxAutoBufferedPaintDC dc(this);
    Draw(dc);
    m_canvasSize = GetSize();
    m_fpsCounter.Update();
}

void Canvas::OnCameraFrameUpdate(wxTimerEvent &event)
{
    if (m_editor.IsCameraFrameUpdated())
    {
        Refresh();
        m_editor.UpdatedFrame();
        m_dialog->OnUpdate();
    }
    auto buttonDisableEvent = m_editor.GetDisabledButtons();
    if (buttonDisableEvent)
        ProcessWindowEvent(buttonDisableEvent.value());
}

void Canvas::OnScreenshot()
{
    auto documentAddEvent = m_editor.OnScreenshot();
    if (documentAddEvent)
    {
        ProcessWindowEvent(documentAddEvent.value());
    }
    m_dialog->OnUpdate();
    ToolboxButtonEvent toolboxEvent(TOOLBOX_BUTTON_COMMAND_DONE, GetId());
    toolboxEvent.SetType(ButtonType::SNAPSHOT_BUTTON);
    ProcessWindowEvent(toolboxEvent);

    auto buttonDisableEvent = m_editor.GetDisabledButtons();
    if (buttonDisableEvent)
        ProcessWindowEvent(buttonDisableEvent.value());
}

void Canvas::OnSave(wxCommandEvent &event)
{
    auto imageSaveEvent = m_editor.OnSave();
    if (imageSaveEvent)
    {
        ProcessWindowEvent(imageSaveEvent.value());
    }
    m_dialog->OnUpdate();
    ToolboxButtonEvent toolboxEvent(TOOLBOX_BUTTON_COMMAND_DONE, GetId());
    toolboxEvent.SetType(ButtonType::SAVE_BUTTON);
    ProcessWindowEvent(toolboxEvent);

    auto buttonDisableEvent = m_editor.GetDisabledButtons();
    if (buttonDisableEvent)
        ProcessWindowEvent(buttonDisableEvent.value());
}

void Canvas::OnDelete(wxCommandEvent &event)
{
    if (m_editor.CanDelete())
    {
        wxMessageDialog dialog(this, "Are you sure you want to delete the selected document?", "Delete document",
                               wxYES_NO | wxICON_QUESTION);
        if (dialog.ShowModal() == wxID_YES)
        {
            auto imageDeleteEvent = m_editor.OnDelete();
            if (imageDeleteEvent)
            {
                ProcessWindowEvent(imageDeleteEvent.value());
            }
            m_dialog->OnUpdate();
            Refresh();
            ToolboxButtonEvent toolboxEvent(TOOLBOX_BUTTON_COMMAND_DONE, GetId());
            toolboxEvent.SetType(ButtonType::DELETE_BUTTON);
            ProcessWindowEvent(toolboxEvent);
            auto buttonDisableEvent = m_editor.GetDisabledButtons();
            if (buttonDisableEvent)
                ProcessWindowEvent(buttonDisableEvent.value());
        }
        else
        {
            ToolboxButtonEvent toolboxEvent(TOOLBOX_BUTTON_COMMAND_DONE, GetId());
            toolboxEvent.SetType(ButtonType::DELETE_BUTTON);
            ProcessWindowEvent(toolboxEvent);
            auto buttonDisableEvent = m_editor.GetDisabledButtons();
            if (buttonDisableEvent)
                ProcessWindowEvent(buttonDisableEvent.value());
        }
    }
}

void Canvas::OnUndo(wxCommandEvent &event)
{
    if (m_editor.CanGoBackToLiveCam())
    {
        if (m_editor.HasAnnotations())
        {
            wxMessageDialog dialog(this, "Are you sure you want to go back?", "Undo", wxYES_NO | wxICON_QUESTION);
            if (dialog.ShowModal() == wxID_YES)
            {
                m_editor.OnUndo();
                m_dialog->OnUpdate();
                Refresh();
                ToolboxButtonEvent toolboxEvent(TOOLBOX_BUTTON_COMMAND_DONE, GetId());
                toolboxEvent.SetType(ButtonType::LIVE_CAMERA_BUTTON);
                ProcessWindowEvent(toolboxEvent);
                auto buttonDisableEvent = m_editor.GetDisabledButtons();
                if (buttonDisableEvent)
                    ProcessWindowEvent(buttonDisableEvent.value());
            }
            else
            {
                ToolboxButtonEvent toolboxEvent(TOOLBOX_BUTTON_COMMAND_DONE, GetId());
                toolboxEvent.SetType(ButtonType::LIVE_CAMERA_BUTTON);
                ProcessWindowEvent(toolboxEvent);
                auto buttonDisableEvent = m_editor.GetDisabledButtons();
                if (buttonDisableEvent)
                    ProcessWindowEvent(buttonDisableEvent.value());
            }
        }
        else
        {
            m_editor.OnUndo();
            m_dialog->OnUpdate();
            Refresh();
            ToolboxButtonEvent toolboxEvent(TOOLBOX_BUTTON_COMMAND_DONE, GetId());
            toolboxEvent.SetType(ButtonType::LIVE_CAMERA_BUTTON);
            ProcessWindowEvent(toolboxEvent);
            auto buttonDisableEvent = m_editor.GetDisabledButtons();
            if (buttonDisableEvent)
                ProcessWindowEvent(buttonDisableEvent.value());
        }
    }
}

void Canvas::OnCancel(wxCommandEvent &event)
{
    m_editor.OnCancel();
    m_dialog->OnUpdate();
    Refresh();
    ToolboxButtonEvent toolboxEvent(TOOLBOX_BUTTON_COMMAND_DONE, GetId());
    toolboxEvent.SetType(ButtonType::SELECT_BUTTON);
    ProcessWindowEvent(toolboxEvent);
}

void Canvas::OnDocumentPicked(ImageDocumentEvent &event)
{
    m_editor.OnDocumentPicked(event.GetData());
    m_dialog->OnUpdate();
    Refresh();
    auto buttonDisableEvent = m_editor.GetDisabledButtons();
    if (buttonDisableEvent)
        ProcessWindowEvent(buttonDisableEvent.value());
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

void Canvas::OnChangeZoomLevel(float scale)
{
    m_editor.ChangeZoomLevel(scale);
    ToolboxButtonEvent toolboxEvent(TOOLBOX_BUTTON_COMMAND_DONE, GetId());
    toolboxEvent.SetType(scale > 1.0f ? ButtonType::ZOOM_IN_BUTTON : ButtonType::ZOOM_OUT_BUTTON);
    ProcessWindowEvent(toolboxEvent);
    Refresh();
}

void Canvas::UpdateAttributeEditor()
{
    auto entities = m_editor.GetSelectedEntities();
    EntityEvent event(EDITOR_ENTITY_CHANGED, wxID_ANY);
    if (!entities.empty())
        event.SetData(entities[0]);

    ProcessWindowEvent(event);
}

InfoDialog::InfoDialog(Canvas *parent, const wxString &title, DrawingSheet &sheet, Editor &editor)
    : wxFrame(parent, wxID_ANY, title), m_sheet(sheet), m_editor(editor), m_canvas(parent)
{
    // Create dialog content
    m_sizer = new wxBoxSizer(wxVERTICAL);
    auto drawStateText = std::format("DrawState:{}", m_sheet.GetDrawState()->GetName());
    auto drawCommandText = std::format("DrawCommand:{}", m_sheet.GetDrawCommandName());
    auto editorState = std::format("EditorState:{}", m_editor.GetStateName());
    auto mousePosition = std::format("MousePosition:{}:{}", m_canvas->GetMousePoint().x, m_canvas->GetMousePoint().y);
    auto canvasSize = std::format("CanvasSize:{}:{}", m_canvas->GetCanvasSize().x, m_canvas->GetCanvasSize().y);
    auto fps = std::format("FPS:{}", m_canvas->GetFPS());

    m_drawState = new wxStaticText(this, wxID_ANY, drawStateText);
    m_drawCommand = new wxStaticText(this, wxID_ANY, drawCommandText);
    m_editorState = new wxStaticText(this, wxID_ANY, editorState);
    m_mousePos = new wxStaticText(this, wxID_ANY, mousePosition);
    m_canvasSize = new wxStaticText(this, wxID_ANY, canvasSize);
    m_fps = new wxStaticText(this, wxID_ANY, fps);

    m_sizer->Add(m_drawState, wxSizerFlags().Align(wxALIGN_TOP).Border(wxALL, FromDIP(1))); // TODO: properly align this
    m_sizer->Add(m_drawCommand, wxSizerFlags().Align(wxALIGN_TOP).Border(wxALL, FromDIP(1)));
    m_sizer->Add(m_editorState, wxSizerFlags().Align(wxALIGN_TOP).Border(wxALL, FromDIP(1)));
    m_sizer->Add(m_mousePos, wxSizerFlags().Align(wxALIGN_TOP).Border(wxALL, FromDIP(1)));
    m_sizer->Add(m_canvasSize, wxSizerFlags().Align(wxALIGN_TOP).Border(wxALL, FromDIP(1)));
    m_sizer->Add(m_fps, wxSizerFlags().Align(wxALIGN_TOP).Border(wxALL, FromDIP(1)));
    SetSizerAndFit(m_sizer);
}

void InfoDialog::OnUpdate()
{
    auto drawStateText = std::format("DrawState:{}", m_sheet.GetDrawState()->GetName());
    auto drawCommandText = std::format("DrawCommand:{}", m_sheet.GetDrawCommandName());
    auto editorStateText = std::format("EditorState:{}", m_editor.GetStateName());
    auto mousePosition = std::format("MousePosition:{}:{}", m_canvas->GetMousePoint().x, m_canvas->GetMousePoint().y);
    auto canvasSize = std::format("CanvasSize:{}:{}", m_canvas->GetCanvasSize().x, m_canvas->GetCanvasSize().y);
    auto fps = std::format("FPS:{}", m_canvas->GetFPS());

    m_drawState->SetLabel(drawStateText);
    m_drawCommand->SetLabel(drawCommandText);
    m_editorState->SetLabel(editorStateText);
    m_mousePos->SetLabel(mousePosition);
    m_canvasSize->SetLabel(canvasSize);
    m_fps->SetLabel(fps);
    Layout();
}

EditorPanel::EditorPanel(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size)
    : wxPanel(parent, id, pos, size)
{
    auto sizer = new wxBoxSizer(wxVERTICAL);
    m_canvas = new Canvas(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);

    auto patientIdInput = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    wxFont patientIdFont(30, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    patientIdInput->SetFont(patientIdFont);
    patientIdInput->Bind(wxEVT_TEXT_ENTER, [this, patientIdInput](wxCommandEvent &event) {
        auto id = patientIdInput->GetValue().ToStdString();
        PatientSelectedEvent patientEvent(EVT_THUMBNAILS_ADD_PATIENT, wxID_ANY);
        patientEvent.SetData(id);
        ProcessWindowEvent(patientEvent);
        patientIdInput->Clear();
    });

    m_statusSizer = new wxBoxSizer(wxVERTICAL);
    m_statusFirstRowSizer = new wxBoxSizer(wxHORIZONTAL);
    std::optional<float> zoomLevel = m_canvas->GetZoomLevel();
    m_patientIdText = new wxStaticText(this, wxID_ANY, "");
    m_patientIdText->SetFont(wxFont(20, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    auto logo = new BitmapPane(wxBitmap("assets/macroshot.png", wxBITMAP_TYPE_PNG), this, wxID_ANY,
      wxTransparentColor, false, {0,0}, {160, 40});
    m_statusSizer->Add(logo, wxSizerFlags(0).Align(wxALIGN_CENTER).Border(wxALL, FromDIP(5)));
    m_statusSizer->Add(m_patientIdText, wxSizerFlags(0).Align(wxALIGN_LEFT).Border(wxALL, FromDIP(5)));
    if (zoomLevel)
    {
        auto formattedText = std::format("Zoom Level:{:.0f}%", (*zoomLevel) * 100.0f);
        m_zoomLevelText = new wxStaticText(this, wxID_ANY, formattedText);
        m_zoomLevelText->SetFont(wxFont(20, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
        m_statusSizer->Add(m_zoomLevelText, wxSizerFlags(0).Border(wxALL, FromDIP(5)));
    }
    sizer->Add(m_statusSizer, wxSizerFlags(0).Expand().Border(wxALL, FromDIP(5)));
    sizer->Add(m_canvas, wxSizerFlags(1).Expand().Border(wxALL, FromDIP(5)));
    sizer->Add(patientIdInput, wxSizerFlags(0).Expand().Border(wxALL, FromDIP(5)));
    SetSizerAndFit(sizer);

    // relay the events from frame to canvas
    Bind(TOOLBOX_BUTTON_PUSHED, [this](ToolboxButtonEvent &event) {
        auto buttonType = event.GetButtonType();
        switch (buttonType)
        {
        case ButtonType::SNAPSHOT_BUTTON:
            m_canvas->OnScreenshot();
            break;
        case ButtonType::SAVE_BUTTON:
            m_canvas->OnSave(event);
            break;
        case ButtonType::DELETE_BUTTON:
            m_canvas->OnDelete(event);
            break;
        case ButtonType::LIVE_CAMERA_BUTTON:
            m_canvas->OnUndo(event);
            break;
        case ButtonType::SELECT_BUTTON:
            m_canvas->OnCancel(event);
            break;
        case ButtonType::DRAW_TEXT_BUTTON:
            m_canvas->OnDrawText(event);
            break;
        case ButtonType::DRAW_ARROW_BUTTON:
            m_canvas->OnDrawArrow(event);
            break;
        case ButtonType::DRAW_LETTERS_BUTTON:
            m_canvas->OnDrawIncrementalLetters(event);
            break;
        case ButtonType::DRAW_CIRCLE_BUTTON:
            m_canvas->OnDrawCircle(event);
            break;
        case ButtonType::DRAW_LINE_BUTTON:
            m_canvas->OnDrawLine(event);
            break;
        case ButtonType::DRAW_MULTILINE_BUTTON:
            m_canvas->OnDrawMultiline(event);
            break;
        case ButtonType::DRAW_RECTANGLE_BUTTON:
            m_canvas->OnDrawRectangle(event);
            break;
        case ButtonType::DRAW_SKIN_TEMPLATE_BUTTON:
            m_canvas->OnDrawSkinTemplate(event);
            break;
        case ButtonType::ZOOM_IN_BUTTON:
            m_canvas->OnChangeZoomLevel(1.2);
            break;
        case ButtonType::ZOOM_OUT_BUTTON:
            m_canvas->OnChangeZoomLevel(1 / 1.2);
            break;
        default:
            wxLogError("Unknown button type");
            break;
        }
        UpdateZoomLevel();
    });
    Bind(EVT_THUMBNAILS_DOCUMENT_PICK, [this](ImageDocumentEvent &event) { m_canvas->OnDocumentPicked(event); });

    Bind(EVT_ENTITY_ATTRIBUTE_EDITED, [this](EntityEvent &event) { wxPostEvent(this->m_canvas, event); });
    Bind(EVT_PATIENT_SELECTED, [this](PatientSelectedEvent &event) {
        auto patientId = event.GetPatientId();
        UpdatePatientId(patientId);
    });
}
void EditorPanel::UpdatePatientId(const std::string &patientId)
{
    if (patientId != "")
    {
        auto formattedText = std::format("Patient: {}", patientId);
        m_patientIdText->SetFont(wxFont(20, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
        m_patientIdText->SetLabel(formattedText);
        m_statusSizer->Layout();
        Refresh();
    }
}
void EditorPanel::UpdateZoomLevel()
{
    std::optional<float> zoomLevel = m_canvas->GetZoomLevel();
    if (zoomLevel)
    {
        auto formattedText = std::format("Zoom Level:{:.0f}%", (*zoomLevel) * 100.0f);

        m_zoomLevelText->SetFont(wxFont(20, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
        m_zoomLevelText->SetLabel(formattedText);
        m_statusSizer->Layout();
        Refresh();
    }
}

} // namespace app
