#include "log.h"
#include <algorithm>
#include <boost/locale/generator.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <cassert>
#include <cctype>
#include <cstdio>
#include <ctime>
#include <cwchar>
#include <fstream>
#include <iostream>
//#include <boost/core/null_deleter.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/attributes/current_thread_id.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/expressions/formatter.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/file.hpp>

#define LOG_MAX_STRING 1024

namespace evnt
{
struct null_deleter
{
    typedef void result_type;
    /*!
     * Does nothing
     */
    template<typename T>
    void operator()(T *) const
    {}
};

namespace Log
{
    /******************************************************************************
    BoostLog
    ******************************************************************************/
    template<typename CharT, typename TraitsT>
    inline std::basic_ostream<CharT, TraitsT> & operator<<(std::basic_ostream<CharT, TraitsT> & strm,
                                                           SeverityLevel                        lvl)
    {
        static const char * const str[] = {"Notification", "WARNING", "ERROR"};
        if(static_cast<std::size_t>(lvl) < (sizeof(str) / sizeof(*str)))
            strm << str[lvl];
        else
            strm << static_cast<int>(lvl);
        return strm;
    }

    static std::string g_fname;
    static Output      g_output;

    using text_sink = boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend>;
    using file_sink = boost::log::sinks::synchronous_sink<boost::log::sinks::text_file_backend>;
    static boost::shared_ptr<text_sink> g_sink_c;
    static boost::shared_ptr<file_sink> g_sink_f;

    static boost::log::sources::wseverity_logger<SeverityLevel> g_wlogger;
    static boost::log::sources::severity_logger<SeverityLevel>  g_logger;

    void BoostLog::InitBoostLog(const std::string & fname, Output out)
    {
        assert(fname.size() > 0);

        boost::log::core::get()->remove_sink(g_sink_c);
        boost::log::core::get()->remove_sink(g_sink_f);

        // static bool once = false;

        g_fname  = fname;
        g_output = out;

        // std::time_t result = std::time( NULL );
        boost::log::formatter format =
            boost::log::expressions::stream
            << boost::log::expressions::attr<unsigned int>("RecordID") << " ["
            << boost::log::expressions::attr<SeverityLevel>("Severity") << "] Thread["
            << boost::log::expressions::attr<boost::log::attributes::current_thread_id::value_type>(
                   "ThreadID")
            //<< "] [" << boost::log::expressions::format_date_time< boost::posix_time::ptime >( "TimeStamp",
            //"%d.%m.%Y %H:%M:%S.%f" )
            << "] Uptime[" << boost::log::expressions::attr<boost::posix_time::time_duration>("Uptime")
            << "] " << boost::log::expressions::smessage;

        // create log file
        if(g_output == all || g_output == file)
        {
            g_sink_f = boost::log::add_file_log(g_fname);

            g_sink_f->locked_backend()->auto_flush(true);

            std::locale loc = boost::locale::generator()("en_US.UTF-8");
            g_sink_f->imbue(loc);

            boost::log::core::get()->add_sink(g_sink_f);
        }

        // console output
        if(g_output == all || g_output == console)
        {
            g_sink_c = boost::make_shared<text_sink>();
            boost::shared_ptr<std::ostream> pStreamCons(&std::clog, null_deleter());
            g_sink_c->locked_backend()->add_stream(pStreamCons);

            g_sink_c->locked_backend()->auto_flush(true);

            std::locale loc = boost::locale::generator()("en_US.UTF-8");
            g_sink_c->imbue(loc);

            boost::log::core::get()->add_sink(g_sink_c);
        }

        /*if(!once)
        {
            BOOST_LOG_SEV( logger_g, Log::notification ) << "Log initialized: " << std::ctime(&result);
            once = true;
        }*/

        if(g_output == all || g_output == file)
        {
            g_sink_f->set_formatter(format);

            g_sink_f->reset_filter();
        }

        if(g_output == all || g_output == console)
        {
            g_sink_c->set_formatter(format);

            g_sink_c->reset_filter();
        }

        boost::log::attributes::counter<unsigned int> RecordID(1);
        boost::log::core::get()->add_global_attribute("RecordID", RecordID);
        boost::log::core::get()->add_global_attribute("ThreadID",
                                                      boost::log::attributes::current_thread_id());
        // boost::log::core::get()->add_global_attribute( "TimeStamp", boost::log::attributes::local_clock()
        // );
        boost::log::core::get()->add_global_attribute("Uptime", boost::log::attributes::timer());
    }

    std::string BoostLog::GetLogFileName() { return g_fname; }

    void BoostLog::SetSeverityFilter(SeverityLevel sl)
    {
        if(g_output == all || g_output == file)
            g_sink_f->set_filter(boost::log::expressions::attr<SeverityLevel>("Severity") >= sl);
        if(g_output == all || g_output == console)
            g_sink_c->set_filter(boost::log::expressions::attr<SeverityLevel>("Severity") >= sl);
    }

    void BoostLog::Put(SeverityLevel sl, const std::string & message)
    {
        BOOST_LOG_SEV(g_logger, sl) << message;
    }

    void BoostLog::Put(SeverityLevel sl, const std::wstring & message)
    {
        BOOST_LOG_SEV(g_wlogger, sl) << message;
    }

    /******************************************************************************
    Global C-style printf fuction
    ******************************************************************************/

    std::string cstr_log(const char * format, ...)
    {
        char buffer[LOG_MAX_STRING] = {};

        // do the formating
        va_list valist;
        va_start(valist, format);
        std::vsnprintf(buffer, sizeof(buffer), format, valist);
        va_end(valist);

        return std::string(buffer);
    }

    std::wstring cstr_log(const wchar_t * format, ...)
    {
        wchar_t buffer[LOG_MAX_STRING] = {};

        // do the formating
        va_list valist;
        va_start(valist, format);
        std::vswprintf(buffer, sizeof(buffer), format, valist);
        va_end(valist);

        return std::wstring(buffer);
    }

    void Log(SeverityLevel sl, const std::wstring & msg) { BoostLog::Put(sl, msg); }

    void Log(SeverityLevel sl, const std::string & msg) { BoostLog::Put(sl, msg); }

    SeverityLevel ConvertStrToSeverity(std::string sl_txt)
    {
        SeverityLevel sl = notification;
        std::transform(sl_txt.begin(), sl_txt.end(), sl_txt.begin(), ::tolower);

        if(sl_txt == std::string("warning"))
            sl = warning;
        if(sl_txt == std::string("error"))
            sl = error;

        return sl;
    }

    Output ConvertStrToOutput(std::string ot_txt)
    {
        Output ot = all;
        std::transform(ot_txt.begin(), ot_txt.end(), ot_txt.begin(), ::tolower);

        if(ot_txt == std::string("console"))
            ot = console;
        if(ot_txt == std::string("file"))
            ot = file;

        return ot;
    }
}   // namespace Log
}   // namespace evnt
