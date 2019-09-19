#include "core.h"
#include "../log/log.h"
#include <boost/property_tree/json_parser.hpp>
#include <chrono>

namespace evnt
{
static std::chrono::steady_clock::time_point app_start{std::chrono::steady_clock::now()};

int64_t GetMilisecFromStart()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - app_start)
        .count();
}

Core::Core()
{
    m_thread_pool  = std::make_unique<ThreadPool>();
    m_event_system = std::make_unique<EventSystem>(*m_thread_pool);

    // http://techgate.fr/boost-property-tree/
    // Load the config.json file in this ptree
    pt::read_json("config.json", m_root_config);

    m_file_system = std::make_unique<FileSystem>(
        m_root_config.get<std::string>("BaseConfig.FileSystem.RootPathRelative"));

    Log::SeverityLevel sl =
        Log::ConvertStrToSeverity(m_root_config.get<std::string>("BaseConfig.Logging.SeverityLevelFilter"));
    Log::Output ot = Log::ConvertStrToOutput(m_root_config.get<std::string>("BaseConfig.Logging.Output"));
    Log::BoostLog::InitBoostLog(m_root_config.get<std::string>("BaseConfig.Logging.FileName"), ot);
    Log::BoostLog::SetSeverityFilter(sl);
}

}   // namespace evnt
