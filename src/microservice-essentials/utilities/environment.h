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
    if(!(stream >> value))
    {
        throw std::invalid_argument(std::string("the value '") + env.value() + " of the environment variable '" + env_name + "' cannot be converted.");
    }
    return std::optional<T>(value);
}

template<typename T> 
T getenv(const std::string& env_name)
{
    std::optional<T> opt_val = mse::getenv_optional<T>(env_name);
    if(!opt_val.has_value())
    {
        throw std::invalid_argument(std::string("environment variable '") + env_name + "' is not set.");
    }
    return opt_val.value();
}

template<typename T> 
T getenv_or(const std::string& env_name, T&& default_value)
{
    std::optional<T> opt_val = mse::getenv_optional<T>(env_name);
    if(!opt_val.has_value())
    {
        return std::move(default_value);
    }
    return opt_val.value();
}

std::string getenv_or(const std::string& env_name, const char* default_value)
{
    return getenv_or(env_name, std::string(default_value));
}

}
