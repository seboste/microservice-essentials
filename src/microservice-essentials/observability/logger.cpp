#include "logger.h"
#include <iostream>
#include <unordered_map>
#include <regex>

using namespace mse;

namespace {

Logger& getDefaultLogger()
{
    static DiscardLogger logger;
    return logger;
}

std::string json_escape(const std::string& str)
{
    static const std::regex escape_regex("[\b\\f\\n\\r\\t\"\\\\]");
    static const std::map<char, std::string> escape_dict = 
    {        
        { '\b' , "\\b" },
        { '\f' , "\\f" },
        { '\n' , "\\n" },
        { '\r' , "\\r" },
        { '\t' , "\\t" },
        { '\"' , "\\\"" },
        { '\\' , "\\\\" }
    };

    std::string escaped_string;
    size_t pos = 0;
    auto escape_begin = std::sregex_iterator(str.begin(), str.end(), escape_regex);    
     for (std::sregex_iterator i = escape_begin; i != std::sregex_iterator(); ++i)
     {
        escaped_string += str.substr(pos, i->position(0) - pos);
        std::string replacement = escape_dict.at(i->str()[0]);
        escaped_string += replacement;
        pos = i->position(0) + 1;
     }

    escaped_string += str.substr(pos, str.size() - pos);
    return escaped_string;
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
    static LogProvider _instance;
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
    _logger = logger;
}

LogProvider::AutoRegistration::AutoRegistration(Logger& logger)
{
    LogProvider::GetInstance().SetLogger(&logger);
}

LogProvider::AutoRegistration::~AutoRegistration()
{
    LogProvider::GetInstance().SetLogger(nullptr);
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
}

Logger::~Logger()
{    
}

ConsoleLogger::ConsoleLogger(LogLevel min_log_level, LogLevel min_err_log_level)
    : Logger(min_log_level)
    , _min_err_log_level(min_err_log_level)
    , _auto_log_provider_registration(*this)
{
    LogProvider::GetInstance().SetLogger(this);
}

ConsoleLogger::~ConsoleLogger()
{
    LogProvider::GetInstance().SetLogger(nullptr);
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


std::string StructuredLogger::to_json(const mse::Context& context, const std::vector<std::string>* fields)
{
    Context::MetadataVector metadata;
    if(fields)
    {
        metadata = context.GetFilteredMetadata(*fields);
    }
    else
    {
        for(const auto& key_value_pair : context.GetAllMetadata())
        {
            metadata.push_back(key_value_pair);
        }
    }
    
    //TODO: escaping. See: https://stackoverflow.com/questions/19176024/how-to-escape-special-characters-in-building-a-json-string
    std::string json  = "{";
    bool is_first = true;
    for(const auto& key_value_pair : metadata)
    {
        if(!is_first)
        {
            json += ",";
        }
        is_first = false;
        json += std::string("\"") +  json_escape(key_value_pair.first) + "\":\"" + json_escape(key_value_pair.second) + "\"";
    }
    json += "}";
    return json;
}

StructuredLogger::StructuredLogger(mse::Logger& logger_backend, std::initializer_list<std::string_view> fields, Formatter formatter)
    : Logger(LogLevel::lowest)
    , _logger_backend(logger_backend)
    , _formatter(formatter)
    , _auto_log_provider_registration(*this)
    , _fields(fields.begin(), fields.end())
{
}
 
StructuredLogger::~StructuredLogger()
{
}

void StructuredLogger::write(const mse::Context& context, mse::LogLevel level, std::string_view message)
{    
    mse::Context context_with_message({ { "message", std::string(message) }, { "level", to_string(level) } }, &context);

    _logger_backend.Write(context_with_message, level, _formatter(context_with_message, _fields.empty() ? nullptr : &_fields));
}

const std::initializer_list<std::string_view> StructuredLogger::default_fields = { "timestamp", "level", "app", "x-b3-traceid", "x-b3-spanid", "message" };
const std::initializer_list<std::string_view> StructuredLogger::all_fields = {};
