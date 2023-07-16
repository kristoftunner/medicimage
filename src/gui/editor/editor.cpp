#include <wx/log.h>

#include "camera/opencv_camera.h"
#include "editor.h"
#include "editor_events.h"
#include "gui/toolbox/toolbox_events.h"
#include "image_handling/image_editor.h"

namespace app
{
using namespace medicimage;

void Editor::Init()
{
    m_cameraFrame = std::make_unique<Image2D>("assets/Checkerboard.png");
    m_camera.Init();
    m_camera.Open(0);

    auto cameraUpdate = [this]() {
        std::chrono::system_clock::time_point lastUpdate = std::chrono::system_clock::now();
        while (true)
        {
            if (m_closeRequested)
            {
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(33));
            {
                if (m_state == EditorState::SHOW_CAMERA)
                {
                    CameraAPI::Frame frame;
                    {
                        std::lock_guard<std::mutex> lock(this->m_cameraMutex);
                        frame = m_camera.CaptureFrame();
                    }

                    if (frame)
                    {
                        std::lock_guard<std::mutex> lock(this->m_cameraFrameMutex);
                        m_cameraFrame = std::move(frame.value());
                        // Create a new bitmap for the region of interest
                        auto zoomWidth = m_cameraFrame->GetWidth() / m_zoomLevel;
                        auto zoomHeight = m_cameraFrame->GetHeight() / m_zoomLevel;
                        float diffx = (m_cameraFrame->GetWidth() - zoomWidth) / 2;
                        float diffy = (m_cameraFrame->GetHeight() - zoomHeight) / 2;
                        wxBitmap roiBitmap(zoomWidth, zoomHeight);
                        wxMemoryDC dc;
                        dc.SelectObject(roiBitmap);
                        dc.DrawBitmap(m_cameraFrame->GetBitmap(), -diffx, -diffy, false);
                        dc.SelectObject(wxNullBitmap);
                        m_cameraFrame->SetBitmap(roiBitmap);
                        m_newFrameAvailable = true;
                    }
                    frame.reset();
                }
            }
            std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed = now - lastUpdate;
            m_cameraFrameRate = 1000.0 / elapsed.count();
        }
    };
    m_cameraUpdateThread = std::thread(cameraUpdate);
}

static std::optional<ButtonType> MapDrawCommandToButton(DrawCommand command)
{
    switch (command)
    {
    case DrawCommand::DRAW_ARROW:
        return ButtonType::DRAW_ARROW_BUTTON;
    case DrawCommand::DRAW_CIRCLE:
        return ButtonType::DRAW_CIRCLE_BUTTON;
    case DrawCommand::DRAW_ELLIPSE:
        return std::nullopt; // TODO: ellipse is not yet implemented
    case DrawCommand::DRAW_INCREMENTAL_LETTERS:
        return ButtonType::DRAW_LETTERS_BUTTON;
    case DrawCommand::DRAW_LINE:
        return ButtonType::DRAW_LINE_BUTTON;
    case DrawCommand::DRAW_MULTILINE:
        return ButtonType::DRAW_MULTILINE_BUTTON;
    case DrawCommand::DRAW_RECTANGLE:
        return ButtonType::DRAW_RECTANGLE_BUTTON;
    case DrawCommand::DRAW_TEXT:
        return ButtonType::DRAW_TEXT_BUTTON;
    case DrawCommand::DRAW_SKIN_TEMPLATE:
        return ButtonType::DRAW_SKIN_TEMPLATE_BUTTON;
    case DrawCommand::DO_NOTHING:
    case DrawCommand::OBJECT_SELECT:
    default:
        return std::nullopt;
    }
}

std::optional<ToolboxButtonEvent> Editor::OnMouseMoved(const glm::vec2 &pos)
{
    if ((m_state == EditorState::EDITING || m_state == EditorState::IMAGE_SELECTION) && m_mouseDown)
    {
        auto clampedPos = ClampMousePosition(pos);
        auto commandReturn = m_drawingSheet.OnMouseButtonDown(clampedPos);
        m_newDrawingAvailable = true;
        if (commandReturn.state == DrawCommandReturn::State::DONE)
        {
            auto buttonType = MapDrawCommandToButton(commandReturn.command);
            if (buttonType)
            {
                auto event = ToolboxButtonEvent(TOOLBOX_BUTTON_COMMAND_DONE, wxID_ANY);
                event.SetType(buttonType.value());
                return event;
            }
        }
    }

    return std::nullopt;
}

std::optional<ToolboxButtonEvent> Editor::OnMousePressed(const glm::vec2 &pos)
{
    if (m_state == EditorState::EDITING || m_state == EditorState::IMAGE_SELECTION)
    {
        m_mouseDown = true;
        auto clampedPos = ClampMousePosition(pos);
        auto commandReturn = m_drawingSheet.OnMouseButtonPressed(clampedPos);
        m_newDrawingAvailable = true;
        if (commandReturn.state == DrawCommandReturn::State::DONE)
        {
            auto buttonType = MapDrawCommandToButton(commandReturn.command);
            if (buttonType)
            {
                auto event = ToolboxButtonEvent(TOOLBOX_BUTTON_COMMAND_DONE, wxID_ANY);
                event.SetType(buttonType.value());
                return event;
            }
        }
    }
    return std::nullopt;
}

std::optional<ToolboxButtonEvent> Editor::OnMouseReleased(const glm::vec2 &pos)
{
    if ((m_state == EditorState::EDITING || m_state == EditorState::IMAGE_SELECTION) && m_mouseDown)
    {
        m_mouseDown = false;
        auto clampedPos = ClampMousePosition(pos);
        auto commandReturn = m_drawingSheet.OnMouseButtonReleased(clampedPos);
        m_newDrawingAvailable = true;
        if (commandReturn.state == DrawCommandReturn::State::DONE)
        {
            auto buttonType = MapDrawCommandToButton(commandReturn.command);
            if (buttonType)
            {
                auto event = ToolboxButtonEvent(TOOLBOX_BUTTON_COMMAND_DONE, wxID_ANY);
                event.SetType(buttonType.value());
                return event;
            }
        }
    }
    return std::nullopt;
}

std::optional<ToolboxButtonEvent> Editor::OnCharInput(const std::string &character)
{
    if (m_state == EditorState::EDITING)
    {
        auto commandReturn = m_drawingSheet.OnTextInput(character);
        m_newDrawingAvailable = true;

        if (commandReturn.state == DrawCommandReturn::State::DONE)
        {
            auto buttonType = MapDrawCommandToButton(commandReturn.command);
            if (buttonType)
            {
                auto event = ToolboxButtonEvent(TOOLBOX_BUTTON_COMMAND_DONE, wxID_ANY);
                event.SetType(buttonType.value());
                return event;
            }
        }
    }
    return std::nullopt;
}

std::optional<ToolboxButtonEvent> Editor::OnKeyPressed(KeyCode key)
{
    if (m_state == EditorState::EDITING)
    {
        auto commandReturn = m_drawingSheet.OnKeyPressed(key);
        m_newDrawingAvailable = true;

        if (commandReturn.state == DrawCommandReturn::State::DONE)
        {
            auto buttonType = MapDrawCommandToButton(commandReturn.command);
            if (buttonType)
            {
                auto event = ToolboxButtonEvent(TOOLBOX_BUTTON_COMMAND_DONE, wxID_ANY);
                event.SetType(buttonType.value());
                return event;
            }
        }
    }

    return std::nullopt;
}

std::optional<ImageDocumentEvent> Editor::OnScreenshot()
{
    if (m_state == EditorState::SHOW_CAMERA)
    {
        wxLogDebug("Changing state SHOW_CAMERA->SCREENSHOT");
        m_state = EditorState::SCREENSHOT;
        std::thread t([this]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            this->OnScreenshotDone();
        });
        t.detach();
        ImageDocumentEvent event(EDITOR_ADD_DOCUMENT, wxID_ANY);
        {
            std::lock_guard<std::mutex> lock(m_cameraFrameMutex);
            event.SetData(ImageDocument(std::make_unique<Image2D>(*(m_cameraFrame.get()))));
        }
        return event;
    }
    else
        return std::nullopt;
}

void Editor::OnScreenshotDone()
{
    if (m_state == EditorState::SCREENSHOT)
    {
        wxLogDebug("Changing state SCREENSHOT->SHOW_CAMERA");
        m_state = EditorState::SHOW_CAMERA;
    }
}

std::optional<ImageDocumentEvent> Editor::OnSave()
{
    if (m_state == EditorState::EDITING)
    {
        wxLogDebug("Changing state EDITING->SHOW_CAMERA");
        m_drawingSheet.SetDrawCommand(DrawCommand::DO_NOTHING);

        // TODO: send the image to the image saver
        ImageDocumentEvent event(EDITOR_SAVE_DOCUMENT, wxID_ANY);
        auto imageWithFooter = Draw();
        auto image = ImageEditor::RemoveFooter(*(imageWithFooter.get()));
        event.SetData(ImageDocument(std::move(image)));
        m_state = EditorState::SHOW_CAMERA;
        return event;
    }
    else
        return std::nullopt;
}

std::optional<ImageDocumentEvent> Editor::OnDelete()
{
    if (m_state == EditorState::IMAGE_SELECTION)
    {
        // TODO: send an event to the image saver to delete this image
        m_state = EditorState::SHOW_CAMERA;
        m_drawingSheet.SetDrawCommand(DrawCommand::DO_NOTHING);

        ImageDocumentEvent event(EDITOR_DELETE_DOCUMENT, wxID_ANY);
        event.SetData(m_activeDocument);
        return event;
    }
    else
        return std::nullopt;
}

bool Editor::CanDelete()
{
    return m_state == EditorState::IMAGE_SELECTION;
}

void Editor::OnUndo()
{
    if (m_state == EditorState::EDITING || m_state == EditorState::IMAGE_SELECTION)
    {
        m_state = EditorState::SHOW_CAMERA;
        m_drawingSheet.SetDrawCommand(DrawCommand::DO_NOTHING);
    }
}

bool Editor::CanGoBackToLiveCam()
{
    return (m_state == EditorState::EDITING || m_state == EditorState::IMAGE_SELECTION);
}

void Editor::OnCancel()
{
    if (m_state == EditorState::EDITING)
    {
        m_drawingSheet.OnCancel();
    }
}

void Editor::OnDrawButtonPushed(DrawCommand command)
{
    if (m_state == EditorState::IMAGE_SELECTION)
    {
        wxLogDebug("Changing state to IMAGE_SELECTION->EDITING");
        m_state = EditorState::EDITING;
        m_drawingSheet.StartAnnotation();
        // auto size = ImageEditor::GetTopleftBorderSize();
        auto borders = ImageEditor::GetImageBorders();
        m_drawingSheet.SetDocument(
            std::make_unique<ImageDocument>(m_activeDocument),
            {m_activeDocument.image->GetWidth() + borders.left + borders.right,
             m_activeDocument.image->GetHeight() + borders.top +
                 borders.bottom}); // TODO REFACTOR: this is just a bad hack, do something with it
        m_drawingSheet.ChangeDrawState(std::make_unique<BaseDrawState>(&m_drawingSheet));
    }
    m_drawingSheet.SetDrawCommand(command);
}

void Editor::OnDocumentPicked(const ImageDocument &document)
{
    if (m_state == EditorState::SHOW_CAMERA || m_state == EditorState::IMAGE_SELECTION)
    {
        m_activeDocument = document;
        auto borders = ImageEditor::GetImageBorders();
        m_drawingSheet.SetDocument(
            std::make_unique<ImageDocument>(m_activeDocument),
            {m_activeDocument.image->GetWidth() + borders.left + borders.right,
             m_activeDocument.image->GetHeight() + borders.top +
                 borders.bottom}); // TODO REFACTOR: this is just a bad hack, do something with it
        m_state = EditorState::IMAGE_SELECTION;
    }
}

std::optional<ToolboButtonStateUpdateEvent> Editor::GetDisabledButtons() const
{
    std::vector<ButtonType> disabledButtons, enabledButtons;
    if (m_state == EditorState::SHOW_CAMERA)
    {
        disabledButtons.push_back(ButtonType::SAVE_BUTTON);
        disabledButtons.push_back(ButtonType::DELETE_BUTTON);
        disabledButtons.push_back(ButtonType::LIVE_CAMERA_BUTTON);
        disabledButtons.push_back(ButtonType::SELECT_BUTTON);
        disabledButtons.push_back(ButtonType::DRAW_TEXT_BUTTON);
        disabledButtons.push_back(ButtonType::DRAW_LETTERS_BUTTON);
        disabledButtons.push_back(ButtonType::DRAW_ARROW_BUTTON);
        disabledButtons.push_back(ButtonType::DRAW_CIRCLE_BUTTON);
        disabledButtons.push_back(ButtonType::DRAW_LINE_BUTTON);
        disabledButtons.push_back(ButtonType::DRAW_MULTILINE_BUTTON);
        disabledButtons.push_back(ButtonType::DRAW_RECTANGLE_BUTTON);
        disabledButtons.push_back(ButtonType::DRAW_SKIN_TEMPLATE_BUTTON);

        enabledButtons.push_back(ButtonType::SNAPSHOT_BUTTON);
        enabledButtons.push_back(ButtonType::ZOOM_IN_BUTTON);
        enabledButtons.push_back(ButtonType::ZOOM_OUT_BUTTON);
    }
    else if (m_state == EditorState::IMAGE_SELECTION)
    {
        disabledButtons.push_back(ButtonType::SNAPSHOT_BUTTON);
        disabledButtons.push_back(ButtonType::SAVE_BUTTON);
        disabledButtons.push_back(ButtonType::SELECT_BUTTON);
        disabledButtons.push_back(ButtonType::ZOOM_IN_BUTTON);
        disabledButtons.push_back(ButtonType::ZOOM_OUT_BUTTON);

        enabledButtons.push_back(ButtonType::DRAW_TEXT_BUTTON);
        enabledButtons.push_back(ButtonType::DRAW_LETTERS_BUTTON);
        enabledButtons.push_back(ButtonType::DRAW_ARROW_BUTTON);
        enabledButtons.push_back(ButtonType::DRAW_CIRCLE_BUTTON);
        enabledButtons.push_back(ButtonType::DRAW_LINE_BUTTON);
        enabledButtons.push_back(ButtonType::DRAW_MULTILINE_BUTTON);
        enabledButtons.push_back(ButtonType::DRAW_RECTANGLE_BUTTON);
        enabledButtons.push_back(ButtonType::DRAW_SKIN_TEMPLATE_BUTTON);
        enabledButtons.push_back(ButtonType::LIVE_CAMERA_BUTTON);
        enabledButtons.push_back(ButtonType::DELETE_BUTTON);
    }
    else if (m_state == EditorState::EDITING)
    {
        disabledButtons.push_back(ButtonType::SNAPSHOT_BUTTON);
        disabledButtons.push_back(ButtonType::DELETE_BUTTON);
        disabledButtons.push_back(ButtonType::ZOOM_IN_BUTTON);
        disabledButtons.push_back(ButtonType::ZOOM_OUT_BUTTON);

        enabledButtons.push_back(ButtonType::SAVE_BUTTON);
        enabledButtons.push_back(ButtonType::LIVE_CAMERA_BUTTON);
        enabledButtons.push_back(ButtonType::SELECT_BUTTON);
        enabledButtons.push_back(ButtonType::DRAW_TEXT_BUTTON);
        enabledButtons.push_back(ButtonType::DRAW_LETTERS_BUTTON);
        enabledButtons.push_back(ButtonType::DRAW_ARROW_BUTTON);
        enabledButtons.push_back(ButtonType::DRAW_CIRCLE_BUTTON);
        enabledButtons.push_back(ButtonType::DRAW_LINE_BUTTON);
        enabledButtons.push_back(ButtonType::DRAW_MULTILINE_BUTTON);
        enabledButtons.push_back(ButtonType::DRAW_RECTANGLE_BUTTON);
        enabledButtons.push_back(ButtonType::DRAW_SKIN_TEMPLATE_BUTTON);
    }

    ToolboButtonStateUpdateEvent event(TOOLBOX_BUTTTON_DISABLED, wxID_ANY);
    event.SetDisabledButtons(disabledButtons);
    event.SetEnabledButtons(enabledButtons);
    return event;
}

void Editor::ChangeZoomLevel(float scale)
{
    double level = m_zoomLevel * scale;
    level = std::clamp(level, 1.0, 15.0);
    m_zoomLevel = static_cast<float>(level);
}

std::unique_ptr<Image2D> Editor::Draw()
{
    switch (m_state)
    {
    case EditorState::SHOW_CAMERA:
    case EditorState::SCREENSHOT: {
        std::lock_guard<std::mutex> lock(m_cameraFrameMutex);
        auto image = std::make_unique<Image2D>(*(m_cameraFrame.get()));
        return std::move(image);
    }
    case EditorState::IMAGE_SELECTION: {
        auto image = m_drawingSheet.Draw();
        return std::move(image);
    }
    case EditorState::EDITING: {
        auto image = m_drawingSheet.Draw();
        return std::move(image);
    }
    default:
        return nullptr;
    }
}

std::string Editor::GetStateName() const
{
    switch (m_state)
    {
    case EditorState::SHOW_CAMERA:
        return "SHOW_CAMERA";
    case EditorState::SCREENSHOT:
        return "SCREENSHOT";
    case EditorState::IMAGE_SELECTION:
        return "IMAGE_SELECTION";
    case EditorState::EDITING:
        return "EDITING";
    default:
        return "UNKNOWN";
    }
}

std::optional<std::string> Editor::GetActiveDocumentName() const
{
    if (m_activeDocument.documentId != "")
        return m_activeDocument.documentId;
    else
        return std::nullopt;
}

std::optional<float> Editor::GetZoomLevel() const
{
    if (m_state == EditorState::SHOW_CAMERA || m_state == EditorState::SCREENSHOT)
    {
        return m_zoomLevel;
    }
    else
        return std::nullopt;
}

glm::vec2 Editor::ClampMousePosition(const glm::vec2 &pos)
{
    const auto imageBorders = ImageEditor::GetImageBorders();
    auto borders = ImageEditor::GetImageBorders();
    const auto imageSize = glm::vec2{m_activeDocument.image->GetWidth(), m_activeDocument.image->GetHeight()};
    const auto maxSize =
        glm::vec2(imageSize.x, imageSize.y) + glm::vec2{borders.left + borders.right, borders.top + borders.bottom};
    const glm::vec2 correctedPos = glm::clamp(pos, glm::vec2{imageBorders.left, imageBorders.top},
                                              maxSize - glm::vec2{borders.right, borders.bottom});
    return correctedPos;
}

void Editor::OnDrawText()
{
    OnDrawButtonPushed(DrawCommand::DRAW_TEXT);
}

void Editor::OnDrawIncrementalLetters()
{
    OnDrawButtonPushed(DrawCommand::DRAW_INCREMENTAL_LETTERS);
}

void Editor::OnDrawArrow()
{
    OnDrawButtonPushed(DrawCommand::DRAW_ARROW);
}

void Editor::OnDrawCircle()
{
    OnDrawButtonPushed(DrawCommand::DRAW_CIRCLE);
}

void Editor::OnDrawLine()
{
    OnDrawButtonPushed(DrawCommand::DRAW_LINE);
}

void Editor::OnDrawMultiline()
{
    OnDrawButtonPushed(DrawCommand::DRAW_MULTILINE);
}

void Editor::OnDrawRectangle()
{
    OnDrawButtonPushed(DrawCommand::DRAW_RECTANGLE);
}

void Editor::OnDrawSkinTemplate()
{
    OnDrawButtonPushed(DrawCommand::DRAW_SKIN_TEMPLATE);
}

} // namespace app
