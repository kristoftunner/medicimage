#pragma once
#include <optional>

#include "image_handling/image_saver.h"
#include "core/utils.h"
#include "editor/editor_events.h"

namespace app
{

wxDECLARE_EVENT(EVT_DOCUMENT_PICKED, ImageDocumentEvent);

using namespace medicimage;

class DocumentController
{
public:
  DocumentController();
  void UpdateAppFolder(const std::filesystem::path& appFolder);
  std::optional<const std::vector<ImageDocument>*> GetSavedImages();
  void SelectPatient(const std::string& patientId);
  std::vector<std::string> GetPatientIds() const; 
  std::string GetSelectedPatientId() const { return m_imageSavers->GetSelectedUuid(); }
  void AddPatient(const std::string& patientId);
  void AddDocument(const ImageDocument& document);

  void OnDeleteDocument(const ImageDocument& document);
private:
  std::unique_ptr<ImageSaverContainer> m_imageSavers;
  AppConfig m_appConfig;
};

} // namespace app
