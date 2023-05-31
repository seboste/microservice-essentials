#pragma once

#include <microservice-essentials/observability/logger.h>

namespace mse_test
{

class HistoryLogger : public mse::Logger
{
public:
  HistoryLogger() : mse::Logger(mse::LogLevel::lowest), _auto_log_provider_registration(*this)
  {
  }
  virtual void write(const mse::Context&, mse::LogLevel level, std::string_view message) override
  {
    _log_history[level].push_back(std::string(message));
  }

  std::map<mse::LogLevel, std::vector<std::string>> _log_history;
  mse::LogProvider::AutoRegistration _auto_log_provider_registration;
};

} // namespace mse_test
