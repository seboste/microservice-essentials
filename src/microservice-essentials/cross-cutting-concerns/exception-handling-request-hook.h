#pragma once

#include <microservice-essentials/cross-cutting-concerns/exception-handling-helpers.h>
#include <microservice-essentials/observability/logger.h>
#include <microservice-essentials/request/request-hook-factory.h>
#include <microservice-essentials/request/request-hook.h>
#include <microservice-essentials/status.h>
#include <vector>

namespace mse
{

/**
 * Request Hook for incoming requests that handles exceptions according to a exception handling definition:
 * - An exception can result in returning a predefined request status.
 * - Optionally the exception details can be forwarded in the status details (best practice for client side errors).
 * - Optionally a log entry with the exception details can be emitted (best practice for internal errors).
 * Note that exception details are only available for exceptions derived from std::exception.
 * The order of exception handling definitions defines the handling priority.
 * Any exception not matched by any of the exception handling definitions will be rethrown.
 */
class ExceptionHandlingRequestHook : public mse::RequestHook
{
public:
  struct Parameters
  {
    Parameters(const std::vector<std::shared_ptr<ExceptionHandling::Mapper>>& exception_handling_mappers =
                   ExceptionHandlingRequestHook::_default_exception_handling_mappers);
    std::vector<std::shared_ptr<ExceptionHandling::Mapper>> exception_handling_mappers =
        ExceptionHandlingRequestHook::_default_exception_handling_mappers;
    AutoRequestHookParameterRegistration<ExceptionHandlingRequestHook::Parameters, ExceptionHandlingRequestHook>
        auto_registration;
  };

  ExceptionHandlingRequestHook(const Parameters& parameters);
  virtual ~ExceptionHandlingRequestHook();

  virtual Status Process(Func func, Context& context) override;

private:
  Parameters _parameters;
  static const std::vector<std::shared_ptr<ExceptionHandling::Mapper>> _default_exception_handling_mappers;
};

} // namespace mse
