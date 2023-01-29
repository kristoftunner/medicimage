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
}

void ImageSaver::ClearSavedImages()
{
  m_savedImages.clear();
}

void ImageSaver::LoadPatientsFolder()
{
  if(!(std::filesystem::create_directory(m_dirPath)))
  {
    APP_CORE_INFO("Directory:{} already created, loading images from it.", m_dirPath.string());
    // iterate trough the files and load the conained images 
    for(auto const& dirEntry : std::filesystem::directory_iterator(m_dirPath))
    {
      if(dirEntry.path().extension() == ".jpeg")
      {
        std::string name = dirEntry.path().stem().string(); 
        LoadImage(name, dirEntry.path());
        APP_CORE_INFO("Picture {} is loaded", dirEntry.path());
      }
    }
  }
}

void ImageSaver::LoadImage(std::string imageName, const std::filesystem::path& filePath)
{
  // assuming there are only one annotated and original variant of an image
  auto findByName = [&](const ImageRef_t& image){return image->GetName() == imageName;};
  
  auto it = std::find_if(m_savedImages.begin(), m_savedImages.end(), findByName);
  if(it == m_savedImages.end())
  { 
    m_savedImages.push_back(std::make_shared<Texture2D>(imageName, filePath.string()));
  }
}

void ImageSaver::SaveImage(std::shared_ptr<Texture2D> texture)
{
  auto in_time_t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::stringstream ss;
  ss << std::put_time(std::localtime(&in_time_t), "%d-%b-%Y %X");
  
  std::string name = m_uuid + "_" + std::to_string(m_savedImages.size());
  texture->SetName(name);

  // need to add footer only to the original image, because for the annotated the original image is used as a base
  std::string footerText = texture->GetName() + " - " + ss.str();
  texture = ImageEditor::AddImageFooter(footerText, texture);

  m_savedImages.push_back(texture);
    
  name += ".jpeg";
  std::filesystem::path imagePath = m_dirPath;
  imagePath /= name; 
    
  cv::UMat ocvImage;
  cv::directx::convertFromD3D11Texture2D(texture->GetTexturePtr(), ocvImage);
  cv::cvtColor(ocvImage, ocvImage, cv::COLOR_RGBA2BGR);
  cv::imwrite(imagePath.string(), ocvImage);
  m_fileLogger->LogFileOperation(name, FileLogger::FileOperation::FILE_SAVE);
}

void ImageSaver::DeleteImage(const std::string& imageName)
{
  auto findByName = [&](const ImageRef_t& image){return image->GetName() == imageName;};

  auto it = std::find_if(m_savedImages.begin(), m_savedImages.end(), findByName);
  if(it != m_savedImages.end())
  {
    auto image = *it;
    std::filesystem::path imagePath = m_dirPath;
    imagePath /= image->GetName() + ".jpeg"; 
    if(!std::filesystem::remove(imagePath))
      APP_CORE_ERR("Something went wrong deleting this image:{}", imagePath.string());
    else
    {
      m_fileLogger->LogFileOperation(image->GetName() + ".jpeg", FileLogger::FileOperation::FILE_DELETE);
      APP_CORE_INFO("Image: {} deleted", imagePath.string());
    }
    m_savedImages.erase(it);
  }
  else
    APP_CORE_ERR("Tried to erase image:{} but not found in the saved images", imageName);
} 

void ImageSaverContainer::AddSaver(const std::string& uuid)
{
  if(uuid != "")
    m_savers[uuid] =  ImageSaver(uuid, m_dataFolder);
  else
    APP_CORE_WARN("Please add valid uuid for patient");
}
  
void ImageSaverContainer::SelectImageSaver(const std::string& uuid)
{
  if(uuid != "")
  {
    // first clear the images saved into memory, add the saver if doesnt exist in the container and select it 
    if(m_selectedSaver != "")
      m_savers[m_selectedSaver].ClearSavedImages();
    if(m_savers.find(uuid) == m_savers.end())
      AddSaver(uuid);
    m_selectedSaver = uuid;
  }
  else
    APP_CORE_WARN("Please select a valid uuid for patient");
}

bool ImageSaverContainer::HasSelectedSaver()
{
  return !m_savers.empty() && m_selectedSaver != "";
}

ImageSaver& ImageSaverContainer::GetSelectedSaver()
{ 
  return m_savers[m_selectedSaver]; 
}

ImageSaverContainer::ImageSaverContainer(const std::filesystem::path& baseFolder)
  : m_dataFolder(baseFolder)
{
  if(!std::filesystem::create_directory(m_dataFolder))
    APP_CORE_INFO("Data folder alredy created, using that one:{}", m_dataFolder);
}

void ImageSaverContainer::UpdateAppFolder(const std::filesystem::path& appFolder)
{
  m_dataFolder = appFolder;
  if (!std::filesystem::create_directory(m_dataFolder))
    APP_CORE_INFO("Directory: {} alredy created, using that one", m_dataFolder);
}

} // namespace medicimage
