#include "error-forwarding-request-hook.h"

using namespace mse;


ErrorForwardingException::ErrorForwardingException(const mse::Status& status)
    : std::runtime_error(status.details)
    , _status(status)
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
        throw ErrorForwardingException(mse::Status{ cit->second, ""});
    }

    return status;
 }
