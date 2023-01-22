#include "image_saver.h"
#include "log.h"

#include "opencv2/core/directx.hpp"
#include "opencv2/core/ocl.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <fstream>
#include <json.hpp>

namespace medicimage
{

using json = nlohmann::json;

void FileLogger::LogFilesave(const std::string& filename)
{
  std::ifstream checkingStream(m_logFileName);
  
  json fileEntry = {
    {"name", filename},
    {"date", "2022.02.23"} // TODO: add date
    };

  json logData; 
  if(!checkingStream.good())
  {
    APP_CORE_INFO("Log file not created for {}, creating log file", m_logFileName);
    std::ofstream creatingStream(m_logFileName);
    json fileEntries = json::array();
    fileEntries.push_back(fileEntry);
    logData["files"] = fileEntries;
    creatingStream.close();
  }
  else
  {
    checkingStream >> logData;
    json files = logData.at("files");
    files.emplace_back(fileEntry);
    logData.at("files") = files;
  }

  const std::string jsonString = logData.dump();
  
  std::ofstream outputFile(m_logFileName);
  if (!outputFile.bad())
  {
    outputFile << jsonString;
    outputFile.close();
  }
  else
    APP_CORE_ERR("Could not dump json into this file:{}", m_logFileName);
} 

ImageSaver::ImageSaver(int uuid, const std::filesystem::path& baseFolder) : m_uuid(uuid), m_dirPath(baseFolder)
{
  m_dirPath /= "data";
  m_dirPath /= std::to_string(uuid);

  if(!(std::filesystem::is_directory(m_dirPath)));
    if(!(std::filesystem::create_directory(m_dirPath)))
      APP_CORE_WARN("Directory:{} already created.", m_dirPath.string());
  m_fileLogger = std::make_unique<FileLogger>(m_dirPath);
}

void ImageSaver::SaveImage(std::shared_ptr<Texture2D> texture, ImageType type)
{
  cv::UMat ocvImage;
  cv::directx::convertFromD3D11Texture2D(texture->GetTexturePtr(), ocvImage);
  cv::cvtColor(ocvImage, ocvImage, cv::COLOR_RGBA2BGR);
  if(type == ImageType::ORIGINAL)
  {
    std::string name = std::to_string(m_uuid) + "_" + std::to_string(m_savedImagePairs.size());
    SavedImagePair imagePair;
    imagePair.name = name;
    imagePair.originalImage = texture;
    m_savedImagePairs.push_back(imagePair);
    texture->SetName(name);
    name += "_original.jpeg";
    std::filesystem::path imagePath = m_dirPath;
    imagePath /= name; 
    cv::imwrite(imagePath.string(), ocvImage);
  }
  else if(type == ImageType::ANNOTATED)
  {
    std::string name = texture->GetName();
    
    auto findByName = [&](const SavedImagePair& imagePair){
      return imagePair.originalImage.value()->GetName() == name;
    };
    auto it = std::find_if(m_savedImagePairs.begin(), m_savedImagePairs.end(), findByName);
    if(it != m_savedImagePairs.end())
    {
      m_savedImagePairs[it - m_savedImagePairs.begin()].annotatedImage = texture; 
      name += "_annotated.jpeg";
      std::filesystem::path imagePath = m_dirPath;
      imagePath /= name; 
      cv::imwrite(imagePath.string(), ocvImage);
    }
    else
      APP_CORE_ERR("Failed to save image:{}", name);
  }
}

void ImageSaver::DeleteImage(const std::string& imageName)
{
  
} 
  
void ImageSaverContainer::SelectImageSaver(const int uuid)
{
  if(m_savers.find(uuid) == m_savers.end())
    m_savers[uuid] =  ImageSaver(uuid, m_baseFolder);
  m_selectedSaver = uuid;
}

bool ImageSaverContainer::IsEmpty()
{
  return m_savers.empty();
}

} // namespace medicimage
