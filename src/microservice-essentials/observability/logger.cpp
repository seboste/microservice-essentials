#include "logger.h"
#include <iostream>

using namespace mse;

namespace {

Logger& getDefaultLogger()
{
    static DiscardLogger logger;
    return logger;
}

}


Logger* Logger::_Instance = nullptr;

Logger& Logger::GetInstance()
{
    if(_Instance)
    {
        return *_Instance;
    }
    else
    {
        return getDefaultLogger();
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

void Logger::SetLogger(Logger* logger)
{
    _Instance = logger;
}

Logger::Logger(LogLevel min_log_level, bool do_registration)
    : _min_log_level(min_log_level)
    , _do_registration(do_registration)
{
    if(_do_registration)
    {
        SetLogger(this);
    }
}

Logger::~Logger()
{
    if(_do_registration)
    {
        SetLogger(nullptr);
    }
}

ConsoleLogger::ConsoleLogger(LogLevel min_log_level, LogLevel min_err_log_level)
    : Logger(min_log_level, true)
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
    : Logger(LogLevel::critical, false)
{
}

DiscardLogger::~DiscardLogger()
{
}

void DiscardLogger::write(const mse::Context& context, mse::LogLevel level, std::string_view message)
{
}


