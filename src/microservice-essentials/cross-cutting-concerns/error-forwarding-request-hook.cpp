#include "error-forwarding-request-hook.h"

using namespace mse;

ErrorForwardingExceptionMapper::ErrorForwardingExceptionMapper(mse::LogLevel loglevel, bool forward_details)    
    : _loglevel(loglevel)
    , _forward_details(forward_details)
{
}

std::optional<mse::ExceptionHandling::Definition> ErrorForwardingExceptionMapper::Map(const std::exception_ptr& exception) const
{
     try
    {
        std::rethrow_exception(exception);
    }
    catch(const ErrorForwardingException& e)
    {
        return mse::ExceptionHandling::Definition { e.GetStatus(), _loglevel, _forward_details && e.GetForwardDetails() };
    }
    catch(...)
    {
        return std::nullopt;
    }
}

ErrorForwardingException::ErrorForwardingException(const mse::Status& status, bool forward_details)
    : std::runtime_error(status.details)
    , _status(status)
    , _forward_details(forward_details)
{
}

ErrorForwardingRequestHook::ErrorForwardingRequestHook(const Parameters& parameters)
    : RequestHook("error forwarding")
    , _parameters(parameters)
{
}

ErrorForwardingRequestHook::~ErrorForwardingRequestHook()
{
}

 Status ErrorForwardingRequestHook::post_process(Context&, Status status)
 {
    const auto cit = _parameters.status_code_mapping.find(status.code);
    if(cit != _parameters.status_code_mapping.end())
    {        
        //TODO: define status details
        throw ErrorForwardingException(mse::Status{ cit->second, ""}, false);
    }

    return status;
 }
