#include "image_handling/image_saver.h"
#include "core/log.h"
#include "image_editor.h"

#include "opencv2/core/directx.hpp"
#include "opencv2/core/ocl.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <fstream>
#include <json.hpp>
#include "image_saver.h"

namespace medicimage
{

using json = nlohmann::json;

void FileLogger::LogFileOperation(const std::string& filename, FileOperation fileOp)
{
  std::string operation = fileOp == FileOperation::FILE_SAVE ? "saved" : "deleted"; 
  json fileEntry = {
    {"name", filename},
    {"date", "2022.02.23"},
    {"operation", operation} 
    };

  json logData;
  std::ifstream logfile(m_logFileName);
  if(!logfile.good())
  {
    logfile.close();
    std::ofstream file(m_logFileName);
    file.close();
  }
  logfile.close();

  logfile.open(m_logFileName);
  if(logfile.good())
  {
    try 
    {
      logfile >> logData;
    }
    catch(const std::exception& e)
    {
      APP_CORE_ERR("Exception:{}", e.what());
    }

    json files;
    if (logData.contains("files"))
    {
      files = logData.at("files");
      files.emplace_back(fileEntry);
    }
    else
    {
      files = json::array();
      files.emplace_back(fileEntry);
    }
    logData["files"] = files;
    const std::string jsonString = logData.dump();
    logfile.close();
    
    std::ofstream outputFile(m_logFileName);
    outputFile << jsonString;
  }
  else
  {
    APP_CORE_ERR("Error writing file operation log into:{}", m_logFileName.string());
  }
} 

ImageSaver::ImageSaver(const std::string& uuid, const std::filesystem::path& baseFolder) : m_uuid(uuid), m_dirPath(baseFolder / uuid)
{
  m_fileLogger = std::make_unique<FileLogger>(m_dirPath);
  CreatePatientDir();
  m_descriptorsFileName = m_dirPath / "documents.json";
}

void ImageSaver::ClearSavedImages()
{
  m_savedImages.clear();
}

void ImageSaver::LoadPatientsFolder()
{
  json jsonData;
  std::ifstream fs(m_descriptorsFileName);
  if(fs.good())
  {
    fs >> jsonData;
    json files = jsonData.at("documents");
    // iterate trough the files and load the conained images 
    for(auto const& dirEntry : std::filesystem::directory_iterator(m_dirPath))
    {
      if(dirEntry.path().extension() == ".jpeg")
      {
        std::string name = dirEntry.path().stem().string(); 
        auto it = std::find_if(files.begin(), files.end(), [&](auto fileDesc)
        {
          auto filename = fileDesc["name"].get<std::string>(); 
          return name == filename; 
        });
        if(it != files.end())
        {
          auto jsonTimeStamp = *it;
          std::string timestampStr = jsonTimeStamp["timestamp"].get<std::string>();
          std::tm t{}; 
          std::istringstream ss(timestampStr);
          ss >> std::get_time(&t, "%d-%b-%Y %X");
          std::time_t timestamp = mktime(&t);
          LoadImage(name, dirEntry.path(), timestamp);
          APP_CORE_INFO("Picture {} is loaded", dirEntry.path());
        }
      }
    }
  }
}

void ImageSaver::CreatePatientDir()
{
  if (!(std::filesystem::create_directory(m_dirPath)))
    APP_CORE_INFO("Directory:{} already created, loading images from it.", m_dirPath.string());
  if(!(std::filesystem::create_directory(m_dirPath/"thumbs")))
    APP_CORE_INFO("thumbs folder alredy created for patient:{}, using that one", m_uuid);

}

void ImageSaver::LoadImage(std::string imageName, const std::filesystem::path& filePath, const std::time_t timestamp)
{
  // TODO: load correctly the document metadata from a meta file
  auto findByName = [&](const ImageDocument& image){return image.documentId == imageName;};
  
  auto it = std::find_if(m_savedImages.begin(), m_savedImages.end(), findByName);
  if(it == m_savedImages.end())
  { 
    m_savedImages.push_back({std::make_unique<Texture2D>(imageName, filePath.string()), filePath.stem().string(), timestamp});
  }
}

void ImageSaver::SaveImage(ImageDocument& doc, bool hasFooter)
{
  // fill out the image timestamp and id only here, because the document should have the timestamp when it is saved 
  std::string name = m_uuid + "_" + std::to_string(m_savedImages.size());
  doc.documentId = name;
  doc.timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

  m_savedImages.push_back(doc);
    
  name += ".jpeg";
  std::filesystem::path imagePath = m_dirPath / name;
  std::filesystem::path thumbImagePath = m_dirPath / "thumbs" / name;
  
  std::stringstream ss;
  ss << std::put_time(std::localtime(&(doc.timestamp)), "%d-%b-%Y %X");
  std::string footerText = doc.documentId + " - " + ss.str();
  auto borderedImage = ImageEditor::AddImageFooter(footerText, doc.texture.get());

  cv::UMat ocvImage;
  cv::directx::convertFromD3D11Texture2D(borderedImage->GetTexturePtr(), ocvImage);
  cv::cvtColor(ocvImage, ocvImage, cv::COLOR_RGBA2BGR);
  cv::imwrite(imagePath.string(), ocvImage);
  cv::resize(ocvImage, ocvImage, cv::Size(640,360) );
  cv::imwrite(thumbImagePath.string(), ocvImage);
  m_fileLogger->LogFileOperation(name, FileLogger::FileOperation::FILE_SAVE);

  // update the json file containing the documents and the timestamp TODO REFACTOR: move this to a function
  json docEntries = json::array();
  for(auto& doc : m_savedImages)
  {
    auto ts = ss.str();
    json docEntry = {{"name", doc.documentId}, {"timestamp", ts}};
    docEntries.push_back(docEntry);
  }
  std::fstream descFile(m_descriptorsFileName, std::ios::in | std::ios::out | std::ios::trunc);
  if (descFile.good())
  {
    json outputJson;
    outputJson["documents"] = docEntries;
    const std::string jsonString = outputJson.dump();
    descFile << jsonString;
  }
  else
  {
    APP_CORE_ERR("Something went wrong with writing to {}", m_descriptorsFileName.string());
  }
}

void ImageSaver::DeleteImage(const std::string& imageName)
{
  auto findByName = [&](const ImageDocument& image){return image.texture->GetName() == imageName;};

  auto it = std::find_if(m_savedImages.begin(), m_savedImages.end(), findByName);
  if(it != m_savedImages.end())
  {
    std::filesystem::path imagePath = m_dirPath / (it->texture->GetName() + ".jpeg");;
    std::filesystem::path thumbImagePath = m_dirPath / "thumbs" / (it->texture->GetName() + ".jpeg");;
    if(!std::filesystem::remove(imagePath) || !(std::filesystem::remove(thumbImagePath)))
      APP_CORE_ERR("Something went wrong deleting this image:{}", imagePath.string());
    else
    {
      m_fileLogger->LogFileOperation(it->texture->GetName() + ".jpeg", FileLogger::FileOperation::FILE_DELETE);
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
    // first clear the images saved into memory, add the saver if doesnt exist in the container, select and load the images 
    if(m_selectedSaver != "")
      m_savers[m_selectedSaver].ClearSavedImages();
    if(m_savers.find(uuid) == m_savers.end())
      AddSaver(uuid);
    m_selectedSaver = uuid;
    m_savers[m_selectedSaver].LoadPatientsFolder();
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

std::unique_ptr<Texture2D> medicimage::ImageDocument::DrawFooter()
{
  std::stringstream ss;
  ss << std::put_time(std::localtime(&timestamp), "%d-%b-%Y %X");
  std::string footerText = documentId + " - " + ss.str();
  return ImageEditor::AddImageFooter(footerText, texture.get());
}

} // namespace medicimage
