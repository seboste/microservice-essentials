#include "health-manager.h"
#include <mutex>
#include <stdexcept>

using namespace mse;

namespace
{

bool getDefaultHealth(HealthManager::HealthType health_type)
{
  switch (health_type)
  {
  case HealthManager::HealthType::liveness:
    return HealthManager::DefaultIsAlive;
  case HealthManager::HealthType::readiness:
    return HealthManager::DefaultIsReady;
  }
  throw std::logic_error("Invalid health type");
}

} // namespace

HealthManager& HealthManager::GetInstance()
{
  static HealthManager instance;
  return instance;
}

void HealthManager::Init(const std::vector<std::string>& liveness_checks,
                         const std::vector<std::string>& readiness_checks)
{
  _health_checks[HealthType::liveness].names = liveness_checks;
  _health_checks[HealthType::readiness].names = readiness_checks;
}

bool HealthManager::IsHealthy(HealthType health_type) const
{
  std::shared_lock<std::shared_mutex> lock(_mutex);
  if (_health_checks.find(health_type) == _health_checks.end())
  {
    return true;
  }

  const auto& health_checks = _health_checks.at(health_type);
  for (const auto& name : health_checks.names)
  {
    if (!isHealthy(health_type, name))
    {
      return false;
    }
  }
  return true;
}

bool HealthManager::IsAlive() const
{
  return IsHealthy(HealthType::liveness);
}

bool HealthManager::IsReady() const
{
  return IsHealthy(HealthType::readiness);
}

bool HealthManager::IsHealthy(HealthType health_type, const std::string& name) const
{
  std::shared_lock<std::shared_mutex> lock(_mutex);
  return isHealthy(health_type, name);
}

bool HealthManager::isHealthy(HealthType health_type, const std::string& name) const
{
  if (_health_checks.find(health_type) == _health_checks.end())
  {
    return getDefaultHealth(health_type);
  }

  const auto& health_checks = _health_checks.at(health_type);
  if (health_checks.callbacks.find(name) == health_checks.callbacks.end())
  {
    return getDefaultHealth(health_type);
  }

  const auto& callback = health_checks.callbacks.at(name);
  if (!callback)
  {
    return getDefaultHealth(health_type);
  }

  return callback();
}

bool HealthManager::IsAlive(const std::string& name) const
{
  return IsHealthy(HealthType::liveness, name);
}
bool HealthManager::IsReady(const std::string& name) const
{
  return IsHealthy(HealthType::readiness, name);
}

bool HealthManager::RegisterHealthCheck(HealthType health_type, const std::string& name,
                                        std::function<bool()> health_check)
{
  std::unique_lock<std::shared_mutex> lock(_mutex);
  auto& health_checks = _health_checks[health_type];
  if (health_checks.callbacks.find(name) != health_checks.callbacks.end())
  {
    // already registered
    return false;
  }

  health_checks.callbacks[name] = health_check;
  return true;
}

void HealthManager::UnregisterHealthCheck(HealthType health_type, const std::string& name)
{
  std::unique_lock<std::shared_mutex> lock(_mutex);
  auto& health_checks = _health_checks[health_type];
  health_checks.callbacks.erase(name);
}
