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

void mse::from_string(const std::string& level_string, LogLevel& level)
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
    
    auto cit = mapping.find(level_string); 
    level = (cit != mapping.cend())
        ? cit->second
        : LogLevel::invalid;    
}

LogProvider& LogProvider::GetInstance()
{
    std::cout << "making sure that the instance exists" << std::endl;
    static LogProvider _instance;
    std::cout << "returning the instance" << std::endl;
    return _instance;
}

Logger& LogProvider::GetLogger()
{
    if(Logger* logger = GetInstance()._logger; logger != nullptr)
    {
        return *logger;
    }
    else
    {
        static DiscardLogger _defaultLogger;
        return _defaultLogger;
    }
}

void LogProvider::SetLogger(Logger* logger)
{
    if(_logger == nullptr
    || logger == nullptr)
    {
        _logger = logger;
    }
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
    level = mse::LogLevel::invalid;
    std::string level_string;
    is >> level_string;
    mse::from_string(level_string, level);
    return level != mse::LogLevel::invalid;
}

std::ostream& operator<<(std::ostream& os, const mse::LogLevel& level)
{
    os << mse::to_string(level);
    return os;
}
