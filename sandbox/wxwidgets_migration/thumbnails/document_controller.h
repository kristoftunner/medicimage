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
  const std::unordered_map<std::string, ImageDocContainer>& GetImageSavers(){return m_imageSavers->GetImageSavers();} // TODO: this is a bit hacky   
  void AddPatient(const std::string& patientId);

  void OnSaveDocument(const ImageDocument& document);
  void OnDeleteDocument(const ImageDocument& document);
private:
  std::unique_ptr<ImageSaverContainer> m_imageSavers;
  AppConfig m_appConfig;
};

} // namespace app
