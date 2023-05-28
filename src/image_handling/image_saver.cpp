#include "image_handling/image_saver.h"
#include "core/log.h"
#include "image_handling/image_editor.h"

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
    json files = json::array();
    json entry = { { "files", files } };
    file << entry;
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

ImageDocContainer::ImageDocContainer(const std::string& uuid, const std::filesystem::path& baseFolder) : m_uuid(uuid), m_dirPath(baseFolder / uuid)
{
  m_fileLogger = std::make_unique<FileLogger>(m_dirPath);
  CreatePatientDir();
  m_descriptorsFileName = m_dirPath / "documents.json";
}

void ImageDocContainer::ClearSavedImages()
{
  m_savedImages.clear();
  APP_CORE_TRACE("Images cleared from patient:{}", m_uuid);
}

void ImageDocContainer::LoadPatientsFolder()
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
          ImageLoad(name, dirEntry.path(), timestamp);
          APP_CORE_TRACE("Picture {} is loaded", dirEntry.path().string());
        }
      }
    }
  }
}

void ImageDocContainer::CreatePatientDir()
{
  if (!(std::filesystem::create_directory(m_dirPath)))
    APP_CORE_INFO("Directory:{} already created, loading images from it.", m_dirPath.string());
  if(!(std::filesystem::create_directory(m_dirPath/"thumbs")))
    APP_CORE_INFO("thumbs folder alredy created for patient:{}, using that one", m_uuid);

}

void ImageDocContainer::ImageLoad(std::string imageName, const std::filesystem::path& filePath, const std::time_t timestamp)
{
  // TODO: load correctly the document metadata from a meta file
  auto findByName = [&](const ImageDocument& image){return image.documentId == imageName;};
  
  auto it = std::find_if(m_savedImages.begin(), m_savedImages.end(), findByName);
  if(it == m_savedImages.end())
  {
    auto image = std::make_unique<Image2D>(filePath.string()); 
    image = std::move(ImageEditor::RemoveFooter(*image.get()));
    m_savedImages.push_back({std::move(image), filePath.stem().string(), timestamp});
  }
}

std::vector<ImageDocument>::iterator ImageDocContainer::AddImage(Image2D& image, bool hasFooter)
{
  json jsonData;
  std::ifstream fs(m_descriptorsFileName);
  int docNumber = 0;
  if(fs.good())
  {
    fs >> jsonData;
    json files = jsonData.at("documents");
    if(files.size() != 0)
    {
      auto lastDocName = files.back()["name"].get<std::string>();
      auto delimiter = lastDocName.find("_") + 1;
      docNumber = std::stoi(lastDocName.substr(delimiter)) + 1;
      APP_CORE_INFO("Last doc name:{}", lastDocName.c_str());
    }
  } 
  std::string name = m_uuid + "_" + std::to_string(docNumber);
  ImageDocument doc(std::make_unique<Image2D>(image));
  doc.documentId = name;
  doc.timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  if(hasFooter)
    doc.image = ImageEditor::RemoveFooter(*doc.image.get());
  m_savedImages.push_back(doc);
    
  name += ".jpeg";
  std::filesystem::path imagePath = m_dirPath / name;
  std::filesystem::path thumbImagePath = m_dirPath / "thumbs" / name;
  
  std::string footerText = doc.GenerateFooterText();
  std::unique_ptr<Image2D> borderedImage;
  borderedImage = ImageEditor::AddImageFooter(footerText, *doc.image.get());

  //cv::UMat ocvImage;
  ////cv::directx::convertFromD3D11Texture2D(borderedImage->GetTexturePtr(), ocvImage);
  //cv::cvtColor(ocvImage, ocvImage, cv::COLOR_RGBA2BGR);
  //cv::imwrite(imagePath.string(), ocvImage);
  //cv::resize(ocvImage, ocvImage, cv::Size(640,360) );
  //cv::imwrite(thumbImagePath.string(), ocvImage);
  image.GetBitmap().SaveFile(imagePath.string(), wxBITMAP_TYPE_PNG);
  auto resizedBitmap = image.GetBitmap().ConvertToImage().Rescale(640, 360);
  resizedBitmap.SaveFile(thumbImagePath.string(), wxBITMAP_TYPE_PNG);
  m_fileLogger->LogFileOperation(name, FileLogger::FileOperation::FILE_SAVE);
  UpdateDocListFile();
  return m_savedImages.end();
}

void ImageDocContainer::DeleteImage(std::vector<ImageDocument>::const_iterator it)
{
  if(it != m_savedImages.end())
  {
    std::filesystem::path imagePath = m_dirPath / (it->documentId + ".jpeg");;
    std::filesystem::path thumbImagePath = m_dirPath / "thumbs" / (it->documentId + ".jpeg");;
    if(!std::filesystem::remove(imagePath) || !(std::filesystem::remove(thumbImagePath)))
      APP_CORE_ERR("Something went wrong deleting this image:{}", imagePath.string());
    else
    {
      m_fileLogger->LogFileOperation(it->documentId + ".jpeg", FileLogger::FileOperation::FILE_DELETE);
      m_savedImages.erase(it);
      UpdateDocListFile();
      APP_CORE_INFO("Image: {} deleted", imagePath.string());
    }
  }
  else
    APP_CORE_ERR("Tried to erase image:{} but not found in the saved images", it->documentId);
}
void ImageDocContainer::UpdateDocListFile()
{
  // update the json file containing the documents and the timestamp TODO REFACTOR: move this to a function
  json docEntries = json::array();
  for(auto& doc : m_savedImages)
  {
    json docEntry = {{"name", doc.documentId}, {"timestamp", doc.GenerateFooterText()}};
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

void ImageSaverContainer::AddSaver(const std::string& uuid)
{
  if(uuid != "")
    m_savers[uuid] =  ImageDocContainer(uuid, m_dataFolder);
  else
    APP_CORE_WARN("Please add valid uuid for patient");
}
  
void ImageSaverContainer::SelectImageSaver(const std::string& uuid)
{
  DeselectImageSaver(); // deselect and flush the loaded images
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

void ImageSaverContainer::DeselectImageSaver()
{
  if(m_selectedSaver != "")
  {
    m_savers[m_selectedSaver].ClearSavedImages();
    m_selectedSaver = "";
  }
}

bool ImageSaverContainer::HasSelectedSaver()
{
  return !m_savers.empty() && m_selectedSaver != "";
}

ImageDocContainer& ImageSaverContainer::GetSelectedSaver()
{ 
  return m_savers[m_selectedSaver]; 
}

ImageSaverContainer::ImageSaverContainer(const std::filesystem::path& baseFolder)
  : m_dataFolder(baseFolder)
{
  if(!std::filesystem::create_directory(m_dataFolder))
  {
    const std::string folderName = m_dataFolder.string();
    APP_CORE_INFO("Data folder alredy created, using that one:{}", folderName);
  }
}

void ImageSaverContainer::UpdateAppFolder(const std::filesystem::path& appFolder)
{
  m_dataFolder = appFolder;
  if (!std::filesystem::create_directory(m_dataFolder))
  {
    const std::string folderName = m_dataFolder.string();
    APP_CORE_INFO("Directory: {} alredy created, using that one", folderName);
  }
}

std::unique_ptr<Image2D> medicimage::ImageDocument::DrawFooter()
{
  std::string footerText = GenerateFooterText();
  return ImageEditor::AddImageFooter(footerText, *image.get());
}

} // namespace medicimage
