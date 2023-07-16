#include "core/utils.h"
#include "core/log.h"
#include "json.hpp"

namespace medicimage
{

using json = nlohmann::json;
// TODO REFACTOR: big big refactor on this file

AppConfig::AppConfig()
{
    std::string configFilename = "config.json";
    std::ifstream checkingStream(configFilename);
    if (!checkingStream.good())
    {
        // APP_CORE_INFO("Log file not created for {}, creating log file", configFilename.c_str());
        std::ofstream configFile(configFilename);
        m_appFolderPath = std::filesystem::current_path();
        json config;
        config = {{"appPath", m_appFolderPath.string()}};
        configFile << config;
        configFile.close();
    }
    else
    {
        json config;
        checkingStream >> config;
        m_appFolderPath = config.at("appPath").get<std::string>();
        if (config.contains("patients"))
        {
            json patients = config.at("patients");
            for (auto &patient : patients)
            {
                PushPatientFolder(patient.get<std::string>());
            }
        }
    }
}

bool AppConfig::UpdateAppFolder(const std::filesystem::path &path)
{
    if (!(std::filesystem::create_directory(path)))
        ;
    // APP_CORE_WARN("Directory:{} already created, using as the app base folder.", path);
    m_appFolderPath = path;
    m_loadedPatientFolders.clear();

    std::filesystem::path configFilePath = m_appFolderPath;
    configFilePath /= std::filesystem::path("config.json");
    std::ifstream configFile(configFilePath.string());
    if (configFile.good())
    {
        json config;
        configFile >> config;
        m_appFolderPath = config.at("appPath").get<std::string>();
        if (config.contains("patients"))
        {
            json patients = config.at("patients");
            for (auto &patient : patients)
            {
                PushPatientFolder(patient.get<std::string>());
            }
        }
    }
    else
    {
        // APP_CORE_INFO("Log file not created for {}, creating log file", configFilename.c_str());
        std::ofstream outputConfigFile(configFilePath.string());
        json config;
        config = {{"appPath", m_appFolderPath.string()}};
        outputConfigFile << config;
        outputConfigFile.close();
    }

    return true;
}

void AppConfig::PushPatientFolder(const std::filesystem::path &patientFolder)
{
    if (m_loadedPatientFolders.size() == 20)
    {
        m_loadedPatientFolders.pop_front();
        m_loadedPatientFolders.push_back(patientFolder);
    }
    else
        m_loadedPatientFolders.push_back(patientFolder);

    // update the json config file
    std::filesystem::path configFilePath = m_appFolderPath;
    configFilePath /= std::filesystem::path("config.json");
    std::ifstream jsonFile(configFilePath.string());
    if (jsonFile.good())
    {
        json config;
        jsonFile >> config;
        json patients = json::array();
        for (auto const &patient : m_loadedPatientFolders)
            patients.emplace_back(patient);
        jsonFile.close();

        std::ofstream outputFile(configFilePath.string());
        config["patients"] = patients;
        outputFile << config;
    }
    else
        APP_CORE_ERR("Application config file not found or corrupted!");
}

} // namespace medicimage
