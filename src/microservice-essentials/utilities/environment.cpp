#include "environment.h"
#include <cstdlib>

namespace mse
{

std::string getenv_or(const std::string& env_name, const char* default_value)
{
    return getenv_or<std::string>(env_name, std::string(default_value));
}

namespace impl
{

std::optional<std::string> getenv(const std::string& env_var)
{
#ifdef _MSC_VER
    char* buf = nullptr;
    size_t sz = 0;
    if (_dupenv_s(&buf, &sz, env_var.c_str()) == 0 && buf != nullptr)
    {        
        std::string env = std::string(buf);        
        free(buf);
        return std::optional<std::string>(env);
    }
    else
    {
        return std::nullopt;
    }
#else
    char* env = std::getenv(env_var.c_str());
    if(env == nullptr)
    {
        return std::nullopt;
    }
    return std::optional<std::string>(std::string(env));
#endif
}

} //impl
} //mse
