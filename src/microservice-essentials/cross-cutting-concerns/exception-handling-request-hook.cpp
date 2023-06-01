#include "exception-handling-request-hook.h"
#include <microservice-essentials/cross-cutting-concerns/error-forwarding-request-hook.h>

namespace
{

std::string extract_exception_details(const std::exception_ptr& exception)
{
  try
  {
    std::rethrow_exception(exception);
  }
  catch (const std::exception& e)
  {
    return e.what();
  }
  catch (...)
  {
  }
  return "";
}

} // namespace

using namespace mse;
using namespace mse::ExceptionHandling;

const std::vector<std::shared_ptr<ExceptionHandling::Mapper>>
    ExceptionHandlingRequestHook::_default_exception_handling_mappers{
        // reasoning for the defaults:
        //- client errors can be forwarded to the client. No logging required.
        //- server errors remain private. Logging is required for diagnosis. Client must know about details.
        //- errors retrieved from depending services are treated as server errors.
        std::make_shared<ErrorForwardingExceptionMapper>(mse::LogLevel::warn, false),
        std::make_shared<ExceptionOfTypeMapper<std::invalid_argument>>(Definition{
            Status{StatusCode::invalid_argument, "invalid argument exception"}, mse::LogLevel::invalid, true}),
        std::make_shared<ExceptionOfTypeMapper<std::out_of_range>>(Definition{
            Status{StatusCode::out_of_range, "out of range argument exception"}, mse::LogLevel::invalid, true}),
        std::make_shared<ToConstantMapper>(
            Definition{Status{StatusCode::internal, "unknown exception"}, mse::LogLevel::warn, false})};

ExceptionHandlingRequestHook::Parameters::Parameters(const std::vector<std::shared_ptr<ExceptionHandling::Mapper>>& ehm)
    : exception_handling_mappers(ehm)
{
}

ExceptionHandlingRequestHook::ExceptionHandlingRequestHook(const Parameters& parameters)
    : RequestHook("exception handling"), _parameters(parameters)
{
}

ExceptionHandlingRequestHook::~ExceptionHandlingRequestHook()
{
}

Status ExceptionHandlingRequestHook::Process(Func func, Context& context)
{
  try
  {
    return func(context);
  }
  catch (...)
  {
    // check all exception types that shall be handled
    for (std::shared_ptr<ExceptionHandling::Mapper> exception_handling_mapper : _parameters.exception_handling_mappers)
    {
      std::optional<mse::ExceptionHandling::Definition> definition =
          exception_handling_mapper->Map(std::current_exception());
      if (definition.has_value())
      {

        if (definition->log_level == mse::LogLevel::invalid && !definition->forward_exception_details_to_caller)
        {
          // exception details are not required => early exit
          return definition->status;
        }

        std::string exception_details = extract_exception_details(std::current_exception());

        if (definition->log_level != mse::LogLevel::invalid)
        {
          mse::LogProvider::GetInstance().GetLogger().Write(definition->log_level,
                                                            std::string("caught exception: ") + exception_details);
        }

        if (definition->forward_exception_details_to_caller)
        {
          mse::Status status = definition->status;
          if (status.details.empty())
          {
            status.details = exception_details;
          }
          else
          {
            status.details += std::string(": ") + exception_details;
          }
          return status;
        }
        else
        {
          return definition->status;
        }
      }
    }
    // rethrow
    throw;
  }
}
