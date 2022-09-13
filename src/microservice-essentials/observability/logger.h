#pragma once

#include <microservice-essentials/context.h>
#include <microservice-essentials/utilities/environment.h>
#include <string_view>

namespace mse
{

enum class LogLevel
{
    //compliant to spdlog
    invalid = -1,
    trace,
    lowest = trace,
    debug,
    info,
    warn,
    err,
    critical,
    highest = critical
};

std::string to_string(LogLevel level);
void from_string(const std::string& level_string, LogLevel& level);

class Logger
{
public:        
    void Write(std::string_view message);
    void Write(LogLevel level, std::string_view message);
    void Write(const Context& context, mse::LogLevel level, std::string_view message);

protected:
    Logger(LogLevel min_log_level);
    virtual ~Logger();
    
    virtual void write(const mse::Context& context, mse::LogLevel level, std::string_view message) = 0;

private:
    LogLevel _min_log_level;    
};

class ConsoleLogger : public mse::Logger
{
public:
    ConsoleLogger(LogLevel min_log_level = mse::getenv_or("LOG_LEVEL", mse::LogLevel::info) , LogLevel min_err_log_level = mse::LogLevel::err);
    virtual ~ConsoleLogger();

    virtual void write(const mse::Context& context, mse::LogLevel level, std::string_view message) override;

private:
    LogLevel _min_err_log_level;
};

class DiscardLogger : public mse::Logger
{
public:
    DiscardLogger();
    virtual ~DiscardLogger();

    virtual void write(const mse::Context& context, mse::LogLevel level, std::string_view message) override;
};

class LogProvider
{
public:
    static LogProvider& GetInstance();
    static Logger& GetLogger();
    
    void SetLogger(Logger* logger);

private:
    LogProvider() = default;
    ~LogProvider() = default;

    Logger* _logger = nullptr;    
};

}

bool operator>>(std::istream& is, mse::LogLevel& level);
std::ostream& operator<<(std::ostream& os, const mse::LogLevel& level);

