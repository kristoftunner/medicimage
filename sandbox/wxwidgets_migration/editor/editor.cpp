#include <wx/log.h>

#include "editor.h"
#include "camera/opencv_camera.h"

namespace app
{
  using namespace medicimage;

void Editor::Init()
{
  m_imageSavers = std::make_unique<ImageSaverContainer>(m_appConfig.GetAppFolder());
  for (const auto &patientFolder : m_appConfig.GetSavedPatientFolders())
    m_imageSavers->AddSaver(patientFolder.stem().string());

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
          auto frame = m_camera.CaptureFrame();
          if (frame)
          {
            std::lock_guard<std::mutex> lock(this->m_cameraFrameMutex);
            this->m_cameraFrame = std::move(frame.value());
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

void Editor::UpdateAppFolder(const std::filesystem::path& appFolder)
{
  m_appConfig.UpdateAppFolder(appFolder);
  m_imageSavers = std::make_unique<ImageSaverContainer>(m_appConfig.GetAppFolder());
  for(const auto& patientFolder : m_appConfig.GetSavedPatientFolders())
    m_imageSavers->AddSaver(patientFolder.stem().string());}

void Editor::OnMouseMoved(const glm::vec2 &pos)
{
  if((m_state == EditorState::EDITING || m_state == EditorState::IMAGE_SELECTION) && m_mouseDown)
  {
    m_drawingSheet.OnMouseButtonDown(pos);
  }
}

void Editor::OnMousePressed(const glm::vec2 &pos)
{
  if(m_state == EditorState::EDITING || m_state == EditorState::IMAGE_SELECTION)
  {
    m_mouseDown = true;
    m_drawingSheet.OnMouseButtonDown(pos);
  }
}

void Editor::OnMouseReleased(const glm::vec2 &pos)
{
  if((m_state == EditorState::EDITING || m_state == EditorState::IMAGE_SELECTION) && m_mouseDown)
  {
    m_mouseDown = false;
    m_drawingSheet.OnMouseButtonReleased(pos);
  }
}

void Editor::OnCharInput(const std::string &character)
{
  if(m_state == EditorState::EDITING)
    m_drawingSheet.OnTextInput(character);
}

void Editor::OnKeyPressed(KeyCode key)
{
  if(m_state == EditorState::EDITING)
    m_drawingSheet.OnKeyPressed(key);
}

void Editor::OnScreenshot()
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
  }
}

void Editor::OnScreenshotDone()
{
  if(m_state == EditorState::SCREENSHOT)
  {
    wxLogDebug("Changing state SCREENSHOT->SHOW_CAMERA");
    m_state = EditorState::SHOW_CAMERA;
  }
}

void Editor::OnSave()
{
  if(m_state == EditorState::EDITING)
  {
    // TODO: add image to image saver
    wxLogDebug("Changing state EDITING->SHOW_CAMERA");
    m_state = EditorState::SHOW_CAMERA;
    m_drawingSheet.SetDrawCommand(DrawCommand::DO_NOTHING);
  }
}

void Editor::OnDelete()
{
  if(m_state == EditorState::IMAGE_SELECTION)
  {
    // TODO: open a modal popup and delete from the image saver
    wxLogDebug("Changing state IMAGE_SELECTION->SHOW_CAMERA");
    m_state = EditorState::SHOW_CAMERA;
  }
}

void Editor::OnUndo()
{
  if(m_state == EditorState::EDITING || m_state == EditorState::IMAGE_SELECTION)
  {
    // TODO: open a modal popup, come back when cancel, set drawcommand to DO_NOTHING and change to SHOW_CAMERA state
  }
}

void Editor::OnImageSelected(int index)
{
  if(m_state == EditorState::SHOW_CAMERA)
  {
    wxLogDebug("Changing state SHOW_CAMERA->IMAGE_SELECTION");
    m_state = EditorState::IMAGE_SELECTION;
  }
}

void Editor::OnDrawButtonPushed(DrawCommand command)
{
  if(m_state == EditorState::IMAGE_SELECTION)
  {
    wxLogDebug("Changing state to IMAGE_SELECTION->EDITING");
    m_state = EditorState::EDITING;
    m_drawingSheet.StartAnnotation();
  }
  m_drawingSheet.SetDrawCommand(command);
}

std::unique_ptr<Image2D> Editor::Draw()
{
  if(m_state == EditorState::SHOW_CAMERA || m_state == EditorState::SCREENSHOT)
  {
    std::lock_guard<std::mutex> lock(m_cameraFrameMutex);
    auto image = std::make_unique<Image2D>(*(m_cameraFrame.get()));
    wxBitmap::Rescale(image->GetBitmap(), {800,600});
    return std::move(image);
  }
  else
  {
    auto image = std::make_unique <Image2D>("Checkerboard.png");
    wxBitmap::Rescale(image->GetBitmap(), {800,600});
    return std::move(image);
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