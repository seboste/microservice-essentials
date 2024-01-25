#pragma once

#include <chrono>
#include <functional>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace mse
{

/**
 * Health manager that can be used to check the health of the application
 * and its dependencies. The health manager is thread-safe.
 */
class HealthManager
{
public:
  enum class HealthType
  {
    liveness, // is the application alive?
    readiness // is the application ready to serve requests?
  };

  static HealthManager& GetInstance();

  void Init(const std::vector<std::string>& liveness_checks, const std::vector<std::string>& readiness_checks);

  bool IsHealthy(HealthType health_type) const;
  bool IsAlive() const;
  bool IsReady() const;

  bool IsHealthy(HealthType health_type, const std::string& name) const;
  bool IsAlive(const std::string& name) const;
  bool IsReady(const std::string& name) const;

  bool RegisterHealthCheck(HealthType health_type, const std::string& name, std::function<bool()> health_check);
  void UnregisterHealthCheck(HealthType health_type, const std::string& name);

  static constexpr bool DefaultIsAlive =
      true; // A microservice is alive by default. A provider must actively set it to false.
  static constexpr bool DefaultIsReady =
      false; // a microservice is not ready by default. A provider must actively set it to true.

private:
  HealthManager() = default;
  virtual ~HealthManager() = default;

  bool isHealthy(HealthType health_type, const std::string& name) const;

  struct HealthChecks
  {
    std::vector<std::string> names;
    std::unordered_map<std::string, std::function<bool()>> callbacks;
  };
  mutable std::shared_mutex _mutex;
  std::unordered_map<HealthType, HealthChecks> _health_checks;
};

/**
 * Health provider that can be used to check the health of the application
 * and its dependencies.
 */
class HealthProvider
{
public:
  HealthProvider(HealthManager::HealthType health_type, const std::string& name, bool is_initially_healthy = false);
  HealthProvider(const HealthProvider&) = delete;
  virtual ~HealthProvider();
  HealthProvider& operator=(const HealthProvider&) = delete;

  void SetHealth(bool is_healthy);
  bool IsHealthy() const;

private:
  bool _is_healthy = false;
};

HealthProvider CreateLivenessProvider(const std::string& name, bool is_initially_alive = false);
HealthProvider CreateReadinessProvider(const std::string& name, bool is_initially_ready = false);

/**
 * Health provider that checks if more than a certain amount of time has passed since the last heartbeat.
 * If that is the case, the health provider is considered unhealthy.
 */
class DeadMansSwitchHealthProvider
{
public:
  using Clock = std::chrono::steady_clock;
  using Duration = std::chrono::duration<Clock>;
  using TimePoint = std::chrono::time_point<Clock>;

  DeadMansSwitchHealthProvider(HealthManager::HealthType health_type, const std::string& name, Duration timeout);
  DeadMansSwitchHealthProvider(const DeadMansSwitchHealthProvider&) = delete;
  virtual ~DeadMansSwitchHealthProvider() = default;
  DeadMansSwitchHealthProvider& operator=(const DeadMansSwitchHealthProvider&) = delete;

  void Notify();
  bool IsHealthy() const;

private:
  const Duration _timeout;
  TimePoint _last_heartbeat;
};

DeadMansSwitchHealthProvider CreateLivenessDeadMansSwitch(const std::string& name,
                                                          DeadMansSwitchHealthProvider::Duration timeout);

DeadMansSwitchHealthProvider CreateReadinessDeadMansSwitch(const std::string& name,
                                                           DeadMansSwitchHealthProvider::Duration timeout);

} // namespace mse
