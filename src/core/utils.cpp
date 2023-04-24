#include "core/utils.h"
#include "core/log.h"
#include "json.hpp"

namespace medicimage
{

using json = nlohmann::json;

AppConfig::AppConfig()
{
  std::string configFilename = "config.json";
  std::ifstream checkingStream(configFilename);
  json config;
  if(!checkingStream.good())
  {
    //APP_CORE_INFO("Log file not created for {}, creating log file", configFilename.c_str());
    std::ofstream configFile(configFilename);
    m_appFolderPath = std::filesystem::current_path();
    config = { 
      {"appPath", m_appFolderPath.string()} 
      };
    configFile << config;
    configFile.close();
  }
  else
  {
    checkingStream >> config;
    m_appFolderPath = config.at("appPath").get<std::string>();
    if(config.contains("patients"))
    {
      json patients = config.at("patients");
      for(auto& patient : patients)
      {
        PushPatientFolder(patient.get<std::string>());
      }
    }
  }
}

bool AppConfig::UpdateAppFolder(const std::filesystem::path& path)
{
  if (!(std::filesystem::create_directory(path)))
    ;
    //APP_CORE_WARN("Directory:{} already created, using as the app base folder.", path);
  m_appFolderPath = path;

  json config;
  std::ifstream configFile("config.json");
  if(configFile.good())
  {
    configFile >> config;
    config["appPath"] = m_appFolderPath.string();
    if(config.contains("patients"))
    {
      config["patients"] = nullptr;
    }
    configFile.close();
    std::ofstream writeConfig("config.json");
    if(writeConfig.good())
      writeConfig << config;
  }

  return true;
}

void AppConfig::PushPatientFolder(const std::filesystem::path& patientFolder)
{
  if(m_loadedPatientFolders.size() == 20)
  {
    m_loadedPatientFolders.pop_front();
    m_loadedPatientFolders.push_back(patientFolder);
  }
  else
    m_loadedPatientFolders.push_back(patientFolder);

  // update the json config file
  std::ifstream jsonFile("config.json");
  if(jsonFile.good())
  {
    json config;
    jsonFile >> config;
    json patients = json::array();
    for(auto const& patient : m_loadedPatientFolders)
      patients.emplace_back(patient);
    jsonFile.close();
    
    std::ofstream outputFile("config.json");
    config["patients"] = patients;
    outputFile << config;
  }
  else
    APP_CORE_ERR("Application config file not found or corrupted!");
}

} // namespace medicimage
