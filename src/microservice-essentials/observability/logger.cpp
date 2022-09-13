#include "logger.h"
#include <iostream>
#include <unordered_map>

using namespace mse;

namespace {

Logger& getDefaultLogger()
{
    static DiscardLogger logger;
    return logger;
}

}

std::string mse::to_string(LogLevel level)
{
    switch(level)
    {
        case LogLevel::trace: return "TRACE";
        case LogLevel::debug: return "DEBUG";
        case LogLevel::info: return "INFO";
        case LogLevel::warn: return "WARN";
        case LogLevel::err: return "ERROR";
        case LogLevel::critical: return "CRITICAL";
    }
    throw std::invalid_argument(std::string("invalid log level with id ") + std::to_string(static_cast<int>(level)));    
}

LogLevel mse::from_string(const std::string& level_string)
{
    static const std::unordered_map<std::string, LogLevel> mapping =
    {
        { "TRACE" , LogLevel::trace },
        { "DEBUG" , LogLevel::debug },
        { "INFO" , LogLevel::info },
        { "WARN" , LogLevel::warn },
        { "ERROR" , LogLevel::err },
        { "CRITICAL" , LogLevel::critical }
    };
    
    if(auto cit = mapping.find(level_string); cit != mapping.cend())
    {
        return cit->second;
    }
    else
    {
        return LogLevel::invalid;
    }
}

LogProvider& LogProvider::GetInstance()
{
    static LogProvider _instance;
    return _instance;
}

Logger& LogProvider::GetLogger()
{
    Logger* logger = GetInstance()._logger;
    return logger != nullptr
        ? *logger
        : GetInstance()._defaultLogger;
}

void LogProvider::SetLogger(Logger* logger)
{
    _logger = logger;
}
    
void Logger::Write(std::string_view message)
{
    Write(LogLevel::info, message);
}

void Logger::Write(LogLevel level, std::string_view message)
{
    //TODO: use default context
    Context ctx;
    Write(ctx, level, message);
}

void Logger::Write(const Context& context, mse::LogLevel level, std::string_view message)
{
    if(static_cast<int>(level) >= static_cast<int>(_min_log_level))
    {
        write(context, level, message);
    }
}

Logger::Logger(LogLevel min_log_level)
    : _min_log_level(min_log_level)
{
    LogProvider::GetInstance().SetLogger(this);    
}

Logger::~Logger()
{
    LogProvider::GetInstance().SetLogger(nullptr);    
}

ConsoleLogger::ConsoleLogger(LogLevel min_log_level, LogLevel min_err_log_level)
    : Logger(min_log_level)
    , _min_err_log_level(min_err_log_level)
{
}

ConsoleLogger::~ConsoleLogger()
{
}

void ConsoleLogger::write(const mse::Context& context, mse::LogLevel level, std::string_view message)
{
    if(static_cast<int>(level) >= static_cast<int>(_min_err_log_level))
    {
        std::cerr << message << std::endl;
    }
    else
    {
        std::cout << message << std::endl;
    }
}


DiscardLogger::DiscardLogger()
    : Logger(LogLevel::critical)
{
}

DiscardLogger::~DiscardLogger()
{
}

void DiscardLogger::write(const mse::Context& context, mse::LogLevel level, std::string_view message)
{
}


bool operator>>(std::istream& is, mse::LogLevel& level)
{
    std::string level_string;
    is >> level_string;
    level = mse::from_string(level_string);
    return level != mse::LogLevel::invalid;
}

std::ostream& operator<<(std::ostream& os, const mse::LogLevel& level)
{
    os << mse::to_string(level);
    return os;
}
