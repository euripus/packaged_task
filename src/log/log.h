#ifndef ILOG_H
#define ILOG_H

#include <string>

/*
Log string sample:
LineID [Severity] [ThreadID] [TimeStamp] [Uptime] Message
*/
namespace evnt
{
namespace Log
{
    enum SeverityLevel
    {
        notification,
        warning,
        error
    };

    enum Output
    {
        all,
        console,
        file
    };

    namespace BoostLog
    {
        void InitBoostLog(const std::string & fname, Output out);

        std::string GetLogFileName();
        void        SetSeverityFilter(SeverityLevel sl);

        void Put(SeverityLevel sl, const std::string & message);
        void Put(SeverityLevel sl, const std::wstring & message);
    }   // namespace BoostLog

    void Log(SeverityLevel sl, const std::wstring & msg);
    void Log(SeverityLevel sl, const std::string & msg);

    // C-style printf fuction
    std::wstring cstr_log(const wchar_t * format, ...);
    std::string  cstr_log(const char * format, ...);

    SeverityLevel ConvertStrToSeverity(std::string sl_txt);
    Output        ConvertStrToOutput(std::string ot_txt);
}   // namespace Log
}   // namespace evnt

#define LOG_MESSAGE(message) evnt::Log::BoostLog::put(evnt::Log::notification, std::string(message));
#define LOG_MESSAGE_W(message) evnt::Log::BoostLog::put(evnt::Log::notification, std::wstring(message));

#define LOG_WARNING(message) evnt::Log::BoostLog::put(evnt::Log::warning, std::string(message));
#define LOG_WARNING_W(message) evnt::Log::BoostLog::put(evnt::Log::warning, std::wstring(message));

#define LOG_ERROR(message) evnt::Log::BoostLog::put(evnt::Log::error, std::string(message));
#define LOG_ERROR_W(message) evnt::Log::BoostLog::put(evnt::Log::error, std::wstring(message));

#endif   // ILOG_H
