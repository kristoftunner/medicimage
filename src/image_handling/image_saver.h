#pragma once

#include <filesystem>
#include "renderer/texture.h"
#include <unordered_map>

namespace medicimage
{


struct SavedImagePair
{
  std::string name;
  std::optional<std::shared_ptr<Image2D>> originalImage;
  std::optional<std::shared_ptr<Image2D>> annotatedImage;
};

class FileLogger
{
public:
  enum class FileOperation{FILE_SAVE, FILE_DELETE};
  FileLogger(const std::filesystem::path& logFileDir) : m_logFileName(logFileDir)
  {
    m_logFileName /= "file_operations.json";
  }
  void LogFileOperation(const std::string& filename, FileOperation fileOp);
  std::vector<std::pair<std::string,std::string>> GetSavedImages(); 
private:
  std::filesystem::path m_logFileName;
};

struct ImageDocument
{
public:  
  ImageDocument() = default;
  ImageDocument(std::unique_ptr<Image2D> im) : image(std::move(im)){} // TODO REFACTOR: these ctors should be refactored in the future
  ImageDocument(std::unique_ptr<Image2D> im, const std::string& id) : image(std::move(im)), documentId(id){}
  ImageDocument(std::unique_ptr<Image2D> im, const std::string& id, const std::time_t& time) : image(std::move(im)), documentId(id), timestamp(time){}
  ImageDocument(const ImageDocument& doc)
  {
    timestamp = doc.timestamp;
    documentId = doc.documentId;
    image = std::make_unique<Image2D>(*doc.image.get());
  }
  ImageDocument& operator=(const ImageDocument& doc)
  {
    timestamp = doc.timestamp;
    documentId = doc.documentId;
    image = std::make_unique<Image2D>(*doc.image.get());
    return *this;
  }
  std::unique_ptr<Image2D> DrawFooter();
  std::string GenerateFooterText()
  {
    std::stringstream ss;
    ss << std::put_time(std::localtime(&(timestamp)), "%d-%b-%Y %X");
    return ss.str();
  }

public:
  std::time_t timestamp = 1683406541;
  std::string documentId = "";
  std::unique_ptr<Image2D> image;
};

class ImageDocContainer
{
  //TODO: implement error handling
public:
  ImageDocContainer() = default;
  ImageDocContainer(const std::string& uuid, const std::filesystem::path& baseFolder);
  // original images are saved only once when doing a screenshot of the image. The original's annotated pair can be replaced multiple
  // times, when it is selected from the thumbnails, edited and then saved as a ANNOTATED image. The original pair can be found
  // by the texture name
  std::vector<ImageDocument>::iterator AddImage(Image2D& image, bool hasFooter);
  void ClearSavedImages();
  void LoadPatientsFolder();
  void CreatePatientDir();
  void ImageLoad(std::string imageName, const std::filesystem::path& filePath, const std::time_t timestamp);
  void DeleteImage(std::vector<ImageDocument>::const_iterator it);
  std::string GetUuid() const {return m_uuid;}
  const std::filesystem::path& GetPatientFolder() { return m_dirPath; }

  // returns a vector of both the original and annotated pair of the image
  const std::vector<ImageDocument>& GetSavedImages(){return m_savedImages;}
private:
  void UpdateDocListFile();
  std::string m_uuid;
  std::filesystem::path m_dirPath;
  std::vector<ImageDocument> m_savedImages;
  std::unique_ptr<FileLogger> m_fileLogger;
  std::filesystem::path m_descriptorsFileName;
};

class ImageSaverContainer
{
public:
  ImageSaverContainer(const std::filesystem::path& baseFolder); 
  void AddSaver(const std::string& uuid);
  void SelectImageSaver(const std::string& uuid);
  void DeselectImageSaver();
  bool HasSelectedSaver();
  void UpdateAppFolder(const std::filesystem::path& appFolder); //TODO: this API call is a bit weird, we should just create a new one in case of changing the app folder
  // API to load the previously saved images from disk at init time of the application
  ImageDocContainer& GetSelectedSaver();
  const std::string& GetSelectedUuid(){return m_selectedSaver;}
  const std::unordered_map<std::string, ImageDocContainer>& GetImageSavers(){return m_savers;} // TODO: this is a bit hacky   
private:
  std::string m_selectedSaver = ""; // uuid of the saver
  std::unordered_map<std::string, ImageDocContainer> m_savers;
  std::filesystem::path m_dataFolder;
};
} // namespace medicimage
