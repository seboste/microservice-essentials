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


/**
 * Abstract base class for all loggers that forwards all messages with at least a minimum log level to the implementation.
 * A single instance of a subclass shall typically be created in the application's main function.
 * The LogProvider singleton shall be used for global access to the logger. 
 */
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

/**
 * Singleton that provides the global logger. 
 * If no logger has been instantiated, a DiscardLogger is used.
 */
class LogProvider
{
public:
    static LogProvider& GetInstance();
    static Logger& GetLogger();
    
    void SetLogger(Logger* logger);

    class AutoRegistration
    {
    public: 
        AutoRegistration(Logger& logger);
        ~AutoRegistration();
    };

private:
    LogProvider() = default;
    ~LogProvider() = default;

    Logger* _logger = nullptr;    
};

/**
 * A logger implementation that writes to standard output/error based on the log level.
 * By default the minimum log level can be specified by setting the 'LOG_LEVEL' environment variable.
 * During construction/destruction of an instance, it is automatically registered/deregistered with the global LogProvider singleton.
 */
class ConsoleLogger : public mse::Logger
{
public:
    ConsoleLogger(LogLevel min_log_level = mse::getenv_or("LOG_LEVEL", mse::LogLevel::info) , LogLevel min_err_log_level = mse::LogLevel::err);
    virtual ~ConsoleLogger();

    virtual void write(const mse::Context& context, mse::LogLevel level, std::string_view message) override;

private:
    LogLevel _min_err_log_level;
    LogProvider::AutoRegistration _auto_log_provider_registration;
};

/**
 * A logger implementation that discards all log messages. Does not automatically register as the global logger.
 */
class DiscardLogger : public mse::Logger
{
public:
    DiscardLogger();
    virtual ~DiscardLogger();

    virtual void write(const mse::Context& context, mse::LogLevel level, std::string_view message) override;
};

} //mse namespace

//stream operators for LogLevel enabling support by mse::getenv
bool operator>>(std::istream& is, mse::LogLevel& level);
std::ostream& operator<<(std::ostream& os, const mse::LogLevel& level);

