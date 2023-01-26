#include "image_saver.h"
#include "log.h"
#include "image_editor.h"

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

void FileLogger::LogFileOperation(const std::string& filename, FileOperation fileOp)
{
  std::ifstream checkingStream(m_logFileName);
  std::string operation = fileOp == FileOperation::FILE_SAVE ? "saved" : "deleted"; 
  json fileEntry = {
    {"name", filename},
    {"date", "2022.02.23"},
    {"operation", operation} 
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

ImageSaver::ImageSaver(const std::string& uuid, const std::filesystem::path& baseFolder) : m_uuid(uuid), m_dirPath(baseFolder)
{
  m_dirPath /= uuid;
  m_fileLogger = std::make_unique<FileLogger>(m_dirPath);

  if(!(std::filesystem::create_directory(m_dirPath)))
  {
    APP_CORE_INFO("Directory:{} already created, loading images from it.", m_dirPath.string());
    // iterate trough the files and load the conained images 
    for(auto const& dirEntry : std::filesystem::directory_iterator(m_dirPath))
    {
      if(dirEntry.path().extension() == ".jpeg")
      {
        std::string name = dirEntry.path().stem().string(); 
        auto origPos = name.find("_original");
        auto annotatedPos = name.find("_annotated");
        if(origPos != std::string::npos)
        {
          LoadImage(name.substr(0, origPos), dirEntry.path(), ImageType::ORIGINAL);
          APP_CORE_INFO("Picture {} is loaded", dirEntry.path());
        }
        else if(annotatedPos != std::string::npos)
        {
          LoadImage(name.substr(0, annotatedPos), dirEntry.path(), ImageType::ANNOTATED);
          APP_CORE_INFO("Picture {} is loaded", dirEntry.path());
        }
      }
    }
  }
}

void ImageSaver::LoadImage(std::string imageName, const std::filesystem::path& filePath, ImageType type)
{
  // assuming there are only one annotated and original variant of an image
  auto findAnnotatedName = [&](const SavedImagePair& imagePair){
    if(imagePair.annotatedImage.has_value())
      return imagePair.annotatedImage.value()->GetName() == imageName;
    else 
      return false;
  };
  
  auto findOriginalName = [&](const SavedImagePair& imagePair){
    if(imagePair.originalImage.has_value())
      return imagePair.originalImage.value()->GetName() == imageName;
    else 
      return false;
  };

  if(type == ImageType::ORIGINAL)
  {
    auto it = std::find_if(m_savedImagePairs.begin(), m_savedImagePairs.end(), findAnnotatedName);
    if(it != m_savedImagePairs.end())
    { 
      // the annotated image is already loaded, so load there the 
      SavedImagePair& imagePair = *it;
      imagePair.originalImage = std::make_shared<Texture2D>(imageName, filePath.string());
    }
    else
    {
      SavedImagePair imagePair;
      imagePair.name = imageName;
      imagePair.originalImage = std::make_shared<Texture2D>(imageName, filePath.string());
      m_savedImagePairs.push_back(imagePair);  
    }
  }
  else if(type == ImageType::ANNOTATED)
  {
    auto it = std::find_if(m_savedImagePairs.begin(), m_savedImagePairs.end(), findOriginalName);
    if(it != m_savedImagePairs.end())
    { 
      // the annotated image is already loaded, so load there the 
      SavedImagePair& imagePair = *it;
      imagePair.annotatedImage = std::make_shared<Texture2D>(imageName, filePath.string());
    }
    else
    {
      SavedImagePair imagePair;
      imagePair.name = imageName;
      imagePair.annotatedImage = std::make_shared<Texture2D>(imageName, filePath.string());
      m_savedImagePairs.push_back(imagePair);  
    }
  }
}

void ImageSaver::SaveImage(std::shared_ptr<Texture2D> texture, ImageType type)
{
  auto in_time_t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::stringstream ss;
  ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
  
  if(type == ImageType::ORIGINAL)
  {
    std::string name = m_uuid + "_" + std::to_string(m_savedImagePairs.size());
    texture->SetName(name);

    // need to add footer only to the original image, because for the annotated the original image is used as a base
    std::string footerText = texture->GetName() + " - " + ss.str();
    texture = ImageEditor::AddImageFooter(footerText, texture);

    SavedImagePair imagePair;
    imagePair.name = name;
    imagePair.originalImage = texture;
    m_savedImagePairs.push_back(imagePair);
    
    name += "_original.jpeg";
    std::filesystem::path imagePath = m_dirPath;
    imagePath /= name; 
    
    cv::UMat ocvImage;
    cv::directx::convertFromD3D11Texture2D(texture->GetTexturePtr(), ocvImage);
    cv::cvtColor(ocvImage, ocvImage, cv::COLOR_RGBA2BGR);
    cv::imwrite(imagePath.string(), ocvImage);
    m_fileLogger->LogFileOperation(name, FileLogger::FileOperation::FILE_SAVE);
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
      
      cv::UMat ocvImage;
      cv::directx::convertFromD3D11Texture2D(texture->GetTexturePtr(), ocvImage);
      cv::cvtColor(ocvImage, ocvImage, cv::COLOR_RGBA2BGR);
      cv::imwrite(imagePath.string(), ocvImage);
      m_fileLogger->LogFileOperation(name, FileLogger::FileOperation::FILE_SAVE);
    }
    else
      APP_CORE_ERR("Failed to save image:{}", name);
  }
}

void ImageSaver::DeleteImage(const std::string& imageName)
{
  auto findByName = [&](const SavedImagePair& imagePair){
    return imagePair.originalImage.value()->GetName() == imageName;};

  auto it = std::find_if(m_savedImagePairs.begin(), m_savedImagePairs.end(), findByName);
  if(it != m_savedImagePairs.end())
  {
    auto imagePair = m_savedImagePairs[it - m_savedImagePairs.begin()];
    if(imagePair.originalImage)
    {
      std::filesystem::path imagePath = m_dirPath;
      imagePath /= imagePair.name + "_original.jpeg"; 
      if(!std::filesystem::remove(imagePath))
        APP_CORE_ERR("Something went wrong deleting this image:{}", imagePath.string());
      else
      {
        m_fileLogger->LogFileOperation(imagePair.name + "_original.jpeg", FileLogger::FileOperation::FILE_DELETE);
        APP_CORE_INFO("Image: {} deleted", imagePath.string());
      }
    }
    if(imagePair.annotatedImage)
    {
      std::filesystem::path imagePath = m_dirPath;
      imagePath /= imagePair.name + "_annotated.jpeg"; 
      if(!std::filesystem::remove(imagePath))
        APP_CORE_ERR("Something went wrong deleting this image:{}", imagePath.string());
      else
      {
        m_fileLogger->LogFileOperation(imagePair.name + "_annotated.jpeg", FileLogger::FileOperation::FILE_DELETE);
        APP_CORE_INFO("Image: {} deleted", imagePath.string());
      }
    }
    m_savedImagePairs.erase(it);
  }
  else
    APP_CORE_ERR("Tried to erase image:{} but not found in the saved images", imageName);
} 
  
void ImageSaverContainer::SelectImageSaver(const std::string& uuid)
{
  if(m_savers.find(uuid) == m_savers.end())
    m_savers[uuid] =  ImageSaver(uuid, m_dataFolder);
  m_selectedSaver = uuid;
}

bool ImageSaverContainer::IsEmpty()
{
  return m_savers.empty();
}

ImageSaverContainer::ImageSaverContainer(const std::filesystem::path& baseFolder)
  : m_dataFolder(baseFolder/"data")
{
  if(!std::filesystem::create_directory(m_dataFolder))
    APP_CORE_INFO("Data folder alredy created, using that one:{}", m_dataFolder);
}

void ImageSaverContainer::UpdateAppFolder(const std::filesystem::path& appFolder)
{
  m_dataFolder = appFolder/"data";
  if (!std::filesystem::create_directory(m_dataFolder))
    APP_CORE_INFO("Directory: {} alredy created, using that one", m_dataFolder);
}

} // namespace medicimage
