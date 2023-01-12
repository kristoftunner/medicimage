#pragma once
#include <memory>
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace medicimage
{
class Logger {
public:
  Logger(){};
  inline static std::shared_ptr<spdlog::logger>& GetClientLogger(){return s_clientLogger;}
  inline static std::shared_ptr<spdlog::logger>& GetCoreLogger(){return s_coreLogger;}
  static void Init();
private:
  static std::shared_ptr<spdlog::logger> s_coreLogger;
  static std::shared_ptr<spdlog::logger> s_clientLogger;
}; 
} // namespace medicimage

// Core logger defines
#define APP_CORE_TRACE(...)  ::medicimage::Logger::GetCoreLogger()->trace(__VA_ARGS__)
#define APP_CORE_INFO(...)   ::medicimage::Logger::GetCoreLogger()->info(__VA_ARGS__)
#define APP_CORE_WARN(...)   ::medicimage::Logger::GetCoreLogger()->warn(__VA_ARGS__)
#define APP_CORE_ERR(...)    ::medicimage::Logger::GetCoreLogger()->error(__VA_ARGS__)

// Client logger defines
#define APP_CLIENT_TRACE(...)  ::medicimage::Logger::GetClientLogger()->trace(__VA_ARGS__)
#define APP_CLIENT_INFO(...)   ::medicimage::Logger::GetClientLogger()->info(__VA_ARGS__)
#define APP_CLIENT_WARN(...)   ::medicimage::Logger::GetClientLogger()->warn(__VA_ARGS__)
#define APP_CLIENT_ERR(...)    ::medicimage::Logger::GetClientLogger()->error(__VA_ARGS__)

