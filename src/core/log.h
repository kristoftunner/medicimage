#pragma once
#include <memory>
#include <wx/wx.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/fmt/ostr.h>

namespace medicimage
{
class WxLogSink : public spdlog::sinks::base_sink<std::mutex>
{
protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {
        // Convert spdlog log level to wxLog level
        wxLogLevel level;
        switch (msg.level)
        {
        case spdlog::level::trace:
            level = wxLOG_Trace;
            break;
        case spdlog::level::debug:
            level = wxLOG_Debug;
            break;
        case spdlog::level::info:
            level = wxLOG_Info;
            break;
        case spdlog::level::warn:
            level = wxLOG_Warning;
            break;
        case spdlog::level::err:
            level = wxLOG_Error;
            break;
        case spdlog::level::critical:
            level = wxLOG_FatalError;
            break;
        default:
            level = wxLOG_Info;
            break;
        }

        // Convert spdlog log message to wxString
        wxString logMsg(msg.payload.data(), wxConvUTF8);

        // Log the message using wxLog
        wxLogMessage(logMsg);
    }

    void flush_() override {}
};

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

