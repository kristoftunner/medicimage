#include "core/log.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace medicimage 
{
  std::shared_ptr<spdlog::logger> Logger::s_coreLogger;
  std::shared_ptr<spdlog::logger> Logger::s_clientLogger;

  void Logger::Init()
  {
    spdlog::set_pattern("%^[%T] %n: %v%$");
    s_coreLogger = spdlog::stdout_color_mt("core");  
    s_clientLogger = spdlog::stdout_color_mt("client"); 
    s_coreLogger->set_level(spdlog::level::info);
    s_clientLogger->set_level(spdlog::level::info); 
  } 
} // namespace gameng

