#pragma once

#include <chrono>
#include <filesystem>

#include "log.h"
namespace medicimage
{

class Timer
{
public:
  Timer(){}
  void Start(int durationMs)
  {
    m_timerDuration = durationMs;
    m_running = true;
    m_startTime = std::chrono::system_clock::now();
  }
  bool Done()
  {
    if(m_running)
    {
      std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
      auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_startTime).count();
      if (elapsedTime >= m_timerDuration)
      {
        m_running = false;
        return true;
      }
      else
        return false;
    }
    else
      return false;
  }
  bool Running(){return m_running;}
private:
  std::chrono::time_point<std::chrono::system_clock> m_startTime;
  int m_timerDuration = 0;
  bool m_running = false;
};

class AppConfig
{
public:
  AppConfig();
  bool SetAppFolder(const std::filesystem::path& path); //TODO: use an exception instead
  const std::filesystem::path& GetAppFolder(){return m_appFolderPath;}
private:
  std::filesystem::path m_appFolderPath;
};

} // namespace medicimage
