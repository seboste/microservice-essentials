#include "error-forwarding-request-hook.h"
#include <sstream>

using namespace mse;



ErrorForwardingRequestHook::Parameters& ErrorForwardingRequestHook::Parameters::IncludeAllErrorCodes(const mse::Status& map_to)
{
    for(int status_code_as_int = static_cast<int>(mse::StatusCode::lowest); status_code_as_int <= static_cast<int>(mse::StatusCode::highest); ++status_code_as_int)
    {
        mse::StatusCode status_code = static_cast<mse::StatusCode>(status_code_as_int);
        if(!mse::Status{status_code, ""})
        {
            //we have some sort of error => map it
            status_code_mapping[status_code] = map_to;
        }        
    }
    return *this;
}

ErrorForwardingRequestHook::Parameters& ErrorForwardingRequestHook::Parameters::Include(mse::StatusCode map_from, const mse::Status& map_to)
{
    status_code_mapping[map_from] = map_to;
    return *this;
}

ErrorForwardingRequestHook::Parameters& ErrorForwardingRequestHook::Parameters::Exclude(mse::StatusCode map_from)
{
    status_code_mapping.erase(map_from);
    return *this;
}       

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
        return mse::ExceptionHandling::Definition { e.GetStatus(), _loglevel, _forward_details };
    }
    catch(...)
    {
        return std::nullopt;
    }
}

ErrorForwardingException::ErrorForwardingException(const mse::Status& status, const std::string& details)
    : std::runtime_error(details)
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

 Status ErrorForwardingRequestHook::post_process(Context& context, Status status)
 {
    const auto cit = _parameters.status_code_mapping.find(status.code);
    if(cit != _parameters.status_code_mapping.end())
    {   
        std::stringstream details_stream;
        details_stream << context.AtOr("request", "") << " received " << to_string(status.code);
        if(!status.details.empty())
        {
            details_stream << "(" << status.details << ")";
        }
        throw ErrorForwardingException(cit->second, details_stream.str() ); //don't forward details
    }
    return status;
 }
