#include "utils.h"
#include "log.h"
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
    config = { {"appPath", m_appFolderPath.string()} };
    configFile << config;
    configFile.close();
  }
  else
  {
    checkingStream >> config;
    m_appFolderPath = config.at("appPath").get<std::string>(); 
  }
}

bool AppConfig::SetAppFolder(const std::filesystem::path& path)
{
  if(!(std::filesystem::create_directory(path)))
    APP_CORE_WARN("Directory:{} already created, using as the app base folder.", path);
  m_appFolderPath = path;

  json config;
  std::ifstream configFile("config.json");
  if(configFile.good())
  {
    configFile >> config;
    config["appPath"] = m_appFolderPath.string(); 
    configFile.close();
    std::ofstream writeConfig("config.json");
    if(writeConfig.good())
      writeConfig << config;
  }

  return true;
}

} // namespace medicimage
