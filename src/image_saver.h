#pragma once

#include <filesystem>
#include "texture.h"
#include <unordered_map>

namespace medicimage
{


struct SavedImagePair
{
  std::string name;
  std::optional<std::shared_ptr<Texture2D>> originalImage;
  std::optional<std::shared_ptr<Texture2D>> annotatedImage;
};

class FileLogger
{
public:
  FileLogger(const std::filesystem::path& logFileDir) : m_logFileName(logFileDir){m_logFileName /= "files.json";}
  void LogFilesave(const std::string& filename);
  std::vector<std::pair<std::string,std::string>> GetSavedImages(); 
private:
  std::filesystem::path m_logFileName;
};

class ImageSaver
{
  //TODO: implement error handling
public:
  enum class ImageType{ORIGINAL, ANNOTATED};
  ImageSaver() = default;
  ImageSaver(const std::string& uuid, const std::filesystem::path& baseFolder);
  // original images are saved only once when doing a screenshot of the image. The original's annotated pair can be replaced multiple
  // times, when it is selected from the thumbnails, edited and then saved as a ANNOTATED image. The original pair can be found
  // by the texture name
  void SaveImage(std::shared_ptr<Texture2D> texture, ImageType type);
  void LoadImage(std::string imageName, const std::filesystem::path& filePath, ImageType type);
  void DeleteImage(const std::string& imageName);
  std::string GetUuid(){return m_uuid;}
  const std::filesystem::path& GetPatientFolder() { return m_dirPath; }

  // returns a vector of both the original and annotated pair of the image
  const std::vector<SavedImagePair>& GetSavedImagePairs(){return m_savedImagePairs;}
private:
  std::string m_uuid;
  std::filesystem::path m_dirPath;
  std::vector<SavedImagePair> m_savedImagePairs;
  std::unique_ptr<FileLogger> m_fileLogger;
};

class ImageSaverContainer
{
public:
  ImageSaverContainer(const std::filesystem::path& baseFolder) : m_baseFolder(baseFolder) {}
  void SelectImageSaver(const std::string& uuid);
  bool IsEmpty();
  // API to load the previously saved images from disk at init time of the application
  ImageSaver& GetSelectedSaver(){ return m_savers[m_selectedSaver]; }
  const std::unordered_map<std::string, ImageSaver>& GetImageSavers(){return m_savers;} // TODO: this is a bit hacky   
private:
  std::string m_selectedSaver; // uuid of the saver
  std::unordered_map<std::string, ImageSaver> m_savers;
  std::filesystem::path m_baseFolder;
};
} // namespace medicimage
