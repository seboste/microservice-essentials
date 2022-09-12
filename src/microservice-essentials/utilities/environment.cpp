#include "environment.h"

namespace mse
{

std::string getenv_or(const std::string& env_name, const char* default_value)
{
    return getenv_or<std::string>(env_name, std::string(default_value));
}

}
