#pragma once

#include <optional>
#include <string>

namespace mse
{

template<typename T> std::optional<T> getenv_optional(const std::string& env_name);
template<typename T> T getenv(const std::string& env_name);
template<typename T> T getenv_or(const std::string& env_name, T&& default_value);
std::string getenv_or(const std::string& env_name, const char* default_value);

}

#include "environment.txx"
