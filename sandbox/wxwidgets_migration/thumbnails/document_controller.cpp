#include "document_controller.h"

namespace app
{

wxDEFINE_EVENT(EVT_DOCUMENT_PICKED, ImageDocumentEvent);

DocumentController::DocumentController()
{
  m_imageSavers = std::make_unique<ImageSaverContainer>(m_appConfig.GetAppFolder());
  for (const auto &patientFolder : m_appConfig.GetSavedPatientFolders())
    m_imageSavers->AddSaver(patientFolder.stem().string());
}

void DocumentController::UpdateAppFolder(const std::filesystem::path &appFolder)
{
  m_appConfig.UpdateAppFolder(appFolder);
  m_imageSavers = std::make_unique<ImageSaverContainer>(m_appConfig.GetAppFolder());
  for(const auto& patientFolder : m_appConfig.GetSavedPatientFolders())
    m_imageSavers->AddSaver(patientFolder.stem().string());
}

std::optional<const std::vector<ImageDocument>*> DocumentController::GetSavedImages()
{
  if (m_imageSavers->HasSelectedSaver())
    return &(m_imageSavers->GetSelectedSaver().GetSavedImages());
  else
    return std::nullopt;
}

void DocumentController::SelectPatient(const std::string &patientId)
{
  m_imageSavers->SelectImageSaver(patientId);
  m_imageSavers->GetSelectedSaver().LoadPatientsFolder();
}

std::vector<std::string> DocumentController::GetPatientIds() const
{
  std::vector<std::string> patientIds;
  for (const auto& saver : m_imageSavers->GetImageSavers())
    patientIds.push_back(saver.first);
  return patientIds;
}

void DocumentController::AddPatient(const std::string &patientId)
{
  // TODO REFACTOR: get rid of these try-catch blocks
  try
  {
    m_imageSavers->SelectImageSaver(patientId);
    m_appConfig.PushPatientFolder(m_imageSavers->GetSelectedSaver().GetPatientFolder());
  }
  catch (std::invalid_argument const& ex)
  {
    APP_CORE_WARN("Please write only numbers for a viable uuid!"); 
  }
  catch (std::out_of_range const& ex)
  {
    APP_CORE_WARN("Please add a number smaller for uuid!"); 
  }
}
void DocumentController::AddDocument(const ImageDocument &document)
{
  if(m_imageSavers->HasSelectedSaver())
    m_imageSavers->GetSelectedSaver().AddImage(*(document.image.get()), false);
}

void DocumentController::OnDeleteDocument(const ImageDocument &document)
{
  if(m_imageSavers->HasSelectedSaver())
    m_imageSavers->GetSelectedSaver().DeleteImage(document);
}
} // namespace app
