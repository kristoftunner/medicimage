#pragma once

#include <filesystem>
#include "texture.h"

namespace medicimage
{

class ImageSaver
{
  //TODO: implement error handling
public:
  ImageSaver(int uuid);
  void SaveImage(std::shared_ptr<Texture2D> texture);
  void DeleteImage(const std::string& imageName);
  int GetUuid(){return m_uuid;} 
  const std::vector<std::shared_ptr<Texture2D>>& GetSavedImages(){ return m_savedImages; } 
private:
  int m_uuid;
  std::filesystem::path m_dirPath;
  std::vector<std::shared_ptr<Texture2D>> m_savedImages;
};

} // namespace medicimage
