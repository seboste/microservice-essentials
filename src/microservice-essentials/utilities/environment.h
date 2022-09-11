#pragma one

#include <cstdlib>
#include <exception>
#include <optional>
#include <sstream>


namespace mse
{

template<typename T> std::optional<T> getenv_optional(const std::string& env_name);
template<typename T> T getenv(const std::string& env_name);
template<typename T> T getenv_or(const std::string& env_name, T&& default_value);

template<>
std::optional<std::string> getenv_optional(const std::string& env_name)
{
    char* env = std::getenv(env_name.c_str());
    if(env == nullptr)
    {
        return std::nullopt;
    }
    return std::optional<std::string>(std::string(env));
}

template<typename T> 
std::optional<T> getenv_optional(const std::string& env_name)
{
    std::optional<std::string> env = mse::getenv_optional<std::string>(env_name);
    if(!env.has_value())
    {
        return std::nullopt;
    }
        
    std::stringstream stream(env.value());
    T value;
    stream >> value;
    return std::optional<T>(value);
}

template<typename T> 
T getenv(const std::string& env_name)
{
    std::optional<T> opt_val = mse::getenv_optional<T>(env_name);
    if(!opt_val.has_value())
    {
        throw std::runtime_error(std::string("environment variable '") + env_name + "' is not set.");
    }
    return opt_val.value();
}

template<typename T> 
T getenv_or(const std::string& env_name, T&& default_value)
{
    std::optional<T> opt_val = mse::getenv_optional<T>(env_name);
    if(!opt_val.has_value())
    {
        return default_value;
    }
    return opt_val.value();
}

}
