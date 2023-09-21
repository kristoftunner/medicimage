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
    std::ifstream appConfigFile(configFilename);
    if (!appConfigFile.good())
    {
        // APP_CORE_INFO("Log file not created for {}, creating log file", configFilename.c_str());
        std::ofstream outputFile(configFilename);
        m_appFolderPath = std::filesystem::current_path();
        json config;
        config = {{"appPath", m_appFolderPath.string()}};
        outputFile << config;
        outputFile.close();
    }
    else
    {
        json appConfig;
        appConfigFile >> appConfig;
        m_appFolderPath = appConfig.at("appPath").get<std::string>();
        std::ifstream patientConfigFile(m_appFolderPath / "patientconfig.json");
        if (!patientConfigFile.good())
        {
            std::ofstream outputConfigFile(m_appFolderPath / "patientconfig.json");
            json patientConfig;
            json patients = json::array();
            patientConfig["patients"] = patients;
            outputConfigFile << patientConfig;
            outputConfigFile.close();
        }
        else
        {
            json patientConfig;
            patientConfigFile >> patientConfig;
            if (patientConfig.contains("patients"))
            {
                json patients = patientConfig.at("patients");
                for (auto &patient : patients)
                {
                    PushPatientFolder(patient.get<std::string>());
                }
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

    std::fstream appConfigFile("config.json");
    if (appConfigFile.good())
    {
        json appConfig;
        appConfigFile >> appConfig;
        appConfig["appPath"] = m_appFolderPath.string();
        appConfigFile.close();

        std::ofstream outputFile("config.json");
        outputFile << appConfig;
        outputFile.close();
    }
    else
        ; // TODO: do some error handling here

    std::filesystem::path patientConfigFilePath = m_appFolderPath / "patientconfig.json";
    std::ifstream patientConfigFile(patientConfigFilePath);
    if (patientConfigFile.good())
    {
        json patientConfig;
        patientConfigFile >> patientConfig;
        if (patientConfig.contains("patients"))
        {
            json patients = patientConfig.at("patients");
            for (auto &patient : patients)
            {
                PushPatientFolder(patient.get<std::string>());
            }
        }
    }
    else
    {
        // APP_CORE_INFO("Log file not created for {}, creating log file", configFilename.c_str());
        json patients = json::array();

        // load patient folders by listing out the directories
        for (auto const &p : std::filesystem::directory_iterator(m_appFolderPath))
        {
            if (p.is_directory())
            {
                patients.emplace_back(p.path().string());
                PushPatientFolder(p.path());
            }
        }

        json patientConfig;
        patientConfig["patients"] = patients;

        std::ofstream outputConfigFile(patientConfigFilePath);
        outputConfigFile << patientConfig;
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
    std::filesystem::path patientConfigFilePath = m_appFolderPath / "patientconfig.json";
    std::ifstream patientConfigFile(patientConfigFilePath);
    if (patientConfigFile.good())
    {
        json patientConfig;
        patientConfigFile >> patientConfig;
        json patients = json::array();
        for (auto const &patient : m_loadedPatientFolders)
            patients.emplace_back(patient);
        patientConfigFile.close();

        std::ofstream outputFile(patientConfigFilePath.string());
        patientConfig["patients"] = patients;
        outputFile << patientConfig;
    }
    else
    {
        std::ofstream outputFile(patientConfigFilePath);
        json patientConfig;
        json patients = json::array();
        for (auto const &patient : m_loadedPatientFolders)
            patients.emplace_back(patient);
        patientConfig["patients"] = patients;
        outputFile << patientConfig;
        outputFile.close();
    }
}

} // namespace medicimage
