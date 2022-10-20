#pragma once

#include <optional>
#include <string>

namespace mse
{

/**
 * Reads an environment variable and converts it to the requested type.
 * Returns nullopt if the environment variable has not been set.
 * Supports all types for which the >> operator is available.
 * Throws std::invalid_argument if the environment variable cannot be converted to the requested type.
 */
template<typename T> std::optional<T> getenv_optional(const std::string& env_name);

/**
 * Reads an environment variable and converts it to the requested type.
 * Throws std::invalid_argument if the environment variable has not been set.
 * Supports all types for which the >> operator is available.
 * Throws std::invalid_argument if the environment variable cannot be converted to the requested type.
 */
template<typename T> T getenv(const std::string& env_name);

/**
 * Reads an environment variable and converts it to the requested type.
 * Returns a default value if the environment variable has not been set.
 * Supports all types for which the >> operator is available.
 * Throws std::invalid_argument if the environment variable cannot be converted to the requested type.
 */
template<typename T> T getenv_or(const std::string& env_name, T&& default_value);
std::string getenv_or(const std::string& env_name, const char* default_value);


namespace impl
{
//wrapper for std::getenv and the MSVC equivalent _dupenv_s (to be used internally only)
std::optional<std::string> getenv(const std::string& env_var); 
}

}

#include "environment.txx"
