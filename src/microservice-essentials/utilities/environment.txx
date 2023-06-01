#pragma once

#include "environment.h"
#include <exception>
#include <sstream>

namespace mse
{

template <> inline std::optional<std::string> getenv_optional(const std::string& env_name)
{
  return mse::impl::getenv(env_name);
}

template <typename T> inline std::optional<T> getenv_optional(const std::string& env_name)
{
  std::optional<std::string> env = mse::getenv_optional<std::string>(env_name);
  if (!env.has_value())
  {
    return std::nullopt;
  }

  std::stringstream stream(env.value());
  T value;
  if (!(stream >> value))
  {
    throw std::invalid_argument(std::string("the value '") + env.value() + " of the environment variable '" + env_name +
                                "' cannot be converted.");
  }
  return std::optional<T>(value);
}

template <typename T> inline T getenv(const std::string& env_name)
{
  std::optional<T> opt_val = mse::getenv_optional<T>(env_name);
  if (!opt_val.has_value())
  {
    throw std::invalid_argument(std::string("environment variable '") + env_name + "' is not set.");
  }
  return opt_val.value();
}

template <typename T> inline T getenv_or(const std::string& env_name, T&& default_value)
{
  std::optional<T> opt_val = mse::getenv_optional<T>(env_name);
  if (!opt_val.has_value())
  {
    return std::move(default_value);
  }
  return opt_val.value();
}

} // namespace mse
