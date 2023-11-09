#include "circuit-breaker-request-hook.h"
#include <microservice-essentials/context.h>
#include <microservice-essentials/observability/logger.h>
#include <mutex>

using namespace mse;

void CircuitBreakerStrategy::pre_process(const Context&)
{
}

void CircuitBreakerStrategy::post_process(const Context&, Status)
{
}

MaxPendingRquestsExceededCircuitBreakerStrategy::MaxPendingRquestsExceededCircuitBreakerStrategy(
    uint32_t max_pending_request_count)
    : _max_pending_request_count(max_pending_request_count)
{
}

MaxPendingRquestsExceededCircuitBreakerStrategy ::~MaxPendingRquestsExceededCircuitBreakerStrategy()
{
}

CircuitBreakerStatus MaxPendingRquestsExceededCircuitBreakerStrategy::GetStatus(const Context& context) const
{
  std::shared_lock<std::shared_mutex> lk(_mutex);
  if (const auto& requestDataCit = _request_data.find(context.AtOr("request", "UNKNOWN"));
      requestDataCit != _request_data.end() &&
      requestDataCit->second._pending_request_count > _max_pending_request_count)
  {
    return CircuitBreakerStatus::OPEN;
  }
  return CircuitBreakerStatus::CLOSED;
}

void MaxPendingRquestsExceededCircuitBreakerStrategy::pre_process(const Context& context)
{
  std::unique_lock<std::shared_mutex> lk(_mutex);
  const std::string request_name = context.AtOr("request", "UNKNOWN");
  uint32_t& pending_request_count = _request_data[request_name]._pending_request_count;
  pending_request_count++;
  if (pending_request_count == _max_pending_request_count + 1)
  {
    MSE_LOG_WARN(std::string("Circuit breaker tripped off for request '") + request_name +
                 "' because number of pending requests exceeds " + std::to_string(_max_pending_request_count));
  }
}

void MaxPendingRquestsExceededCircuitBreakerStrategy::post_process(const Context& context, Status /*status*/)
{
  std::unique_lock<std::shared_mutex> lk(_mutex);
  const std::string request_name = context.AtOr("request", "UNKNOWN");
  uint32_t& pending_request_count = _request_data[request_name]._pending_request_count;
  pending_request_count--;
  if (pending_request_count == _max_pending_request_count)
  {
    MSE_LOG_INFO(std::string("Circuit breaker returned to normal state (CLOSED) for request '" + request_name));
  }
}

CircuitBreakerRequestHook::Parameters::Parameters(std::shared_ptr<CircuitBreakerStrategy> strat, const mse::Status& es)
    : strategy(strat), errorStatus(es)
{
}

CircuitBreakerRequestHook::CircuitBreakerRequestHook(const Parameters& parameters)
    : RequestHook("circuit breaker"), _parameters(parameters)
{
}

Status CircuitBreakerRequestHook::pre_process(Context& context)
{
  _parameters.strategy->pre_process(context);
  switch (_parameters.strategy->GetStatus(context))
  {
  case CircuitBreakerStatus::CLOSED:
    return Status::OK;
  case CircuitBreakerStatus::OPEN:
  default:
    // make sure that post_process is called even if circuit breaker is open and
    // the call to the actual function is not made
    _parameters.strategy->post_process(context, _parameters.errorStatus);
    return _parameters.errorStatus;
  }
  return _parameters.errorStatus;
}

Status CircuitBreakerRequestHook::post_process(Context& context, Status status)
{
  _parameters.strategy->post_process(context, status);
  return status;
}
