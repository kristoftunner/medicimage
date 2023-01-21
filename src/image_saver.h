#pragma once

#include <filesystem>
#include "texture.h"
#include <unordered_map>

namespace medicimage
{


class FileLogger
{
public:
  FileLogger(const std::filesystem::path& logFileDir) : m_logFileName(logFileDir){m_logFileName /= "files.json";}
  void LogFilesave(const std::string& filename); 
private:
  std::filesystem::path m_logFileName;
};

class ImageSaver
{
  //TODO: implement error handling
public:
  ImageSaver() = default;
  ImageSaver(int uuid);
  void SaveImage(std::shared_ptr<Texture2D> texture);
  void DeleteImage(const std::string& imageName);
  int GetUuid(){return m_uuid;} 
  const std::vector<std::shared_ptr<Texture2D>>& GetSavedImages(){ return m_savedImages; } 
private:
  int m_uuid;
  std::filesystem::path m_dirPath;
  std::vector<std::shared_ptr<Texture2D>> m_savedImages;
  std::unique_ptr<FileLogger> m_fileLogger;
};

class ImageSaverContainer
{
public:
  ImageSaverContainer() = default;
  void SelectImageSaver(const int uuid);
  bool IsEmpty();
  ImageSaver& GetSelectedSaver(){ return m_savers[m_selectedSaver]; }
private:
  int m_selectedSaver; // uuid of the saver
  std::unordered_map<int, ImageSaver> m_savers;
};
} // namespace medicimage
