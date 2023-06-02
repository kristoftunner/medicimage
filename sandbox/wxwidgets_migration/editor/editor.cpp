#include <wx/log.h>

#include "editor.h"
#include "editor_events.h"
#include "camera/opencv_camera.h"
#include "image_handling/image_editor.h"

namespace app
{
using namespace medicimage;

void Editor::Init()
{
  m_cameraFrame = std::make_unique<Image2D>("Checkerboard.png");
  m_camera.Init();
  m_camera.Open(0);

  auto cameraUpdate = [this](){
    std::chrono::system_clock::time_point lastUpdate = std::chrono::system_clock::now();
    while(true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(33));
      {
        if(this->m_state == EditorState::SHOW_CAMERA)
        {
          CameraAPI::Frame frame;
          {
            std::lock_guard<std::mutex> lock(this->m_cameraMutex);
            frame = m_camera.CaptureFrame();
          }

          if (frame)
          {
            std::lock_guard<std::mutex> lock(this->m_cameraFrameMutex);
            this->m_cameraFrame = std::move(frame.value());
            wxBitmap::Rescale(this->m_cameraFrame->GetBitmap(), {800, 600});
            this->m_newFrameAvailable = true;
          }
          frame.reset();
        }
      }
      std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
      std::chrono::duration<double> elapsed = now - lastUpdate;
      this->m_cameraFrameRate = 1000.0 / elapsed.count();
    }
  };
  m_cameraUpdateThread = std::thread(cameraUpdate);
}

void Editor::OnMouseMoved(const glm::vec2 &pos)
{
  if((m_state == EditorState::EDITING || m_state == EditorState::IMAGE_SELECTION) && m_mouseDown)
  {
    m_drawingSheet.OnMouseButtonDown(pos);
    m_newDrawingAvailable = true;
  }
}

void Editor::OnMousePressed(const glm::vec2 &pos)
{
  if(m_state == EditorState::EDITING || m_state == EditorState::IMAGE_SELECTION)
  {
    m_mouseDown = true;
    m_drawingSheet.OnMouseButtonPressed(pos);
    m_newDrawingAvailable = true;
  }
}

void Editor::OnMouseReleased(const glm::vec2 &pos)
{
  if((m_state == EditorState::EDITING || m_state == EditorState::IMAGE_SELECTION) && m_mouseDown)
  {
    m_mouseDown = false;
    m_drawingSheet.OnMouseButtonReleased(pos);
    m_newDrawingAvailable = true;
  }
}

void Editor::OnCharInput(const std::string &character)
{
  if(m_state == EditorState::EDITING)
  {
    m_drawingSheet.OnTextInput(character);
    m_newDrawingAvailable = true;
  }
}

void Editor::OnKeyPressed(KeyCode key)
{
  if(m_state == EditorState::EDITING)
  {
    m_drawingSheet.OnKeyPressed(key);
    m_newDrawingAvailable = true;
  }
}

std::optional<ImageDocumentEvent> Editor::OnScreenshot()
{
  if(m_state == EditorState::SHOW_CAMERA)
  {
    wxLogDebug("Changing state SHOW_CAMERA->SCREENSHOT");
    m_state = EditorState::SCREENSHOT;
    std::thread t([this](){
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
      this->OnScreenshotDone();
    });
    t.detach();
    ImageDocumentEvent event(EVT_EDITOR_ADD_DOCUMENT, wxID_ANY);
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
  if(m_state == EditorState::SCREENSHOT)
  {
    wxLogDebug("Changing state SCREENSHOT->SHOW_CAMERA");
    m_state = EditorState::SHOW_CAMERA;
  }
}

std::optional<ImageDocumentEvent> Editor::OnSave()
{
  if(m_state == EditorState::EDITING)
  {
    wxLogDebug("Changing state EDITING->SHOW_CAMERA");
    m_drawingSheet.SetDrawCommand(DrawCommand::DO_NOTHING);

    // TODO: send the image to the image saver
    ImageDocumentEvent event(EVT_EDITOR_SAVE_DOCUMENT, wxID_ANY);
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
  if(m_state == EditorState::IMAGE_SELECTION)
  {
    // TODO: send an event to the image saver to delete this image
    m_state = EditorState::SHOW_CAMERA;
    m_drawingSheet.SetDrawCommand(DrawCommand::DO_NOTHING);

    ImageDocumentEvent event(EVT_EDITOR_DELETE_DOCUMENT, wxID_ANY);
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
  if(m_state == EditorState::EDITING || m_state == EditorState::IMAGE_SELECTION)
  {
    m_state = EditorState::SHOW_CAMERA;
    m_drawingSheet.SetDrawCommand(DrawCommand::DO_NOTHING);
  }
}

bool Editor::CanUndo() 
{
  return (m_state == EditorState::EDITING || m_state == EditorState::IMAGE_SELECTION) && m_drawingSheet.HasAnnotated();
}

void Editor::OnDrawButtonPushed(DrawCommand command)
{
  if(m_state == EditorState::IMAGE_SELECTION)
  {
    wxLogDebug("Changing state to IMAGE_SELECTION->EDITING");
    m_state = EditorState::EDITING;
    m_drawingSheet.StartAnnotation();
    auto size = ImageEditor::GetTopleftBorderSize();
    m_drawingSheet.SetDocument(std::make_unique<ImageDocument>(m_activeDocument), {m_activeDocument.image->GetWidth() + 2*size.x, m_activeDocument.image->GetHeight() + 60}); // TODO REFACTOR: this is just a bad hack, do something with it
    m_drawingSheet.ChangeDrawState(std::make_unique<BaseDrawState>(&m_drawingSheet));
  }
  m_drawingSheet.SetDrawCommand(command);
}

void Editor::OnDocumentPicked(const ImageDocument &document)
{
  if(m_state == EditorState::SHOW_CAMERA || m_state == EditorState::IMAGE_SELECTION)
  {
    m_activeDocument = document;
    m_drawingSheet.SetDocument(std::make_unique<ImageDocument>(m_activeDocument), {m_activeDocument.image->GetWidth(), m_activeDocument.image->GetHeight()});   
    m_state = EditorState::IMAGE_SELECTION;
  }
}

std::unique_ptr<Image2D> Editor::Draw()
{
  switch(m_state)
  {
    case EditorState::SHOW_CAMERA:
    case EditorState::SCREENSHOT:
    {
      std::lock_guard<std::mutex> lock(m_cameraFrameMutex);
      auto image = std::make_unique<Image2D>(*(m_cameraFrame.get()));
      return std::move(image);
    }
    case EditorState::IMAGE_SELECTION:
    {
      auto image = m_drawingSheet.Draw(); // needed because add footer is called in the Draw method
      return std::move(image);
    }
    case EditorState::EDITING:
    {
      auto image = m_drawingSheet.Draw();
      return std::move(image);
    }
  }
}

std::string Editor::GetStateName() const
{
  switch(m_state)
  {
    case EditorState::SHOW_CAMERA: return "SHOW_CAMERA";
    case EditorState::SCREENSHOT: return "SCREENSHOT";
    case EditorState::IMAGE_SELECTION: return "IMAGE_SELECTION";
    case EditorState::EDITING: return "EDITING";
    default: return "UNKNOWN";
  }
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