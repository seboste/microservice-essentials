#pragma once

#include <memory>
#include <microservice-essentials/request/request-hook-factory.h>
#include <microservice-essentials/request/request-hook.h>
#include <shared_mutex>
#include <unordered_map>

namespace mse
{

enum class CircuitBreakerStatus
{
  CLOSED, // Normal operation
  OPEN    // Request will not be issued
};

/// abstract backoff strategy for CircuitBreakerRequestHook
class CircuitBreakerStrategy
{
public:
  virtual ~CircuitBreakerStrategy() = default;

  /**
   * Returns whether the circuit breaker is open or closed
   */
  virtual CircuitBreakerStatus GetStatus(const Context& context) const = 0;

protected:
  // will be called by CircuitBreakerRequestHook to allow
  friend class CircuitBreakerRequestHook;
  virtual void pre_process(const Context& context);
  virtual void post_process(const Context& context, Status status);
};

class MaxPendingRquestsExceededCircuitBreakerStrategy : public CircuitBreakerStrategy
{
public:
  MaxPendingRquestsExceededCircuitBreakerStrategy(uint32_t max_pending_request_count);
  virtual ~MaxPendingRquestsExceededCircuitBreakerStrategy();

  /// @see CircuitBreakerStrategy
  virtual CircuitBreakerStatus GetStatus(const Context& context) const override;

protected:
  virtual void pre_process(const Context& context) override;
  virtual void post_process(const Context& context, Status status) override;

private:
  uint32_t _max_pending_request_count;
  struct RequestData
  {
    uint32_t _pending_request_count = 0;
  };
  std::unordered_map<std::string, RequestData> _request_data; // request-name => request-data
  mutable std::shared_mutex _mutex;
};

/**
 * Request hook for outgoing requests that returns immediately (without even issueing the request) and returns an
 * error code in case a circuit breaker strategy (e.g. the number of pending outgoing requests exceeds a certain
 * threshold) decides that the receiver will likely not be able to handle the request.
 */
class CircuitBreakerRequestHook : public mse::RequestHook
{
public:
  struct Parameters
  {
    Parameters(std::shared_ptr<CircuitBreakerStrategy> strat,
               const mse::Status& es = mse::Status{mse::StatusCode::unavailable, "circuit breaker tripped off"});

    std::shared_ptr<CircuitBreakerStrategy> strategy;
    mse::Status errorStatus;
    AutoRequestHookParameterRegistration<CircuitBreakerRequestHook::Parameters, CircuitBreakerRequestHook>
        auto_registration;
  };

  CircuitBreakerRequestHook(const Parameters& parameters);
  virtual ~CircuitBreakerRequestHook() = default;

protected:
  virtual Status pre_process(Context& context) override;
  virtual Status post_process(Context& context, Status status) override;

private:
  Parameters _parameters;
};

} // namespace mse
