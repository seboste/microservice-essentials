#include "logging-request-hook.h"
#include <microservice-essentials/request/request-hook-factory.h>

using namespace mse;

/*
namespace {

std::unique_ptr<mse::RequestHook> CreateLoggingRequestHook(const std::any& parameters)
{
    //return std::make_unique<LoggingRequestHook>(std::any_cast<LoggingRequestHook::Parameters>(parameters));
}

//execute this registration at some point during static initialization
static bool register_hook = []()
{    
    //mse::RequestHookFactory::GetInstance().Register<LoggingRequestHook::Parameters>(CreateLoggingRequestHook);
    return true;
}();

}

*/

LoggingRequestHook::LoggingRequestHook(const Parameters& parameters)
    : RequestHook("Logging")
    , _parameters(parameters)
{    
}

Status LoggingRequestHook::pre_process(Context& context)
{
    mse::LogProvider::GetLogger().Write(context, _parameters.loglevel_success, 
        get_request_verb_pre() + " request " + context.AtOr("request", "unknown")
    );
    return Status::OK;
}
 
Status LoggingRequestHook::post_process(Context& context, Status status)
{
    mse::LogProvider::GetLogger().Write(context, status ? _parameters.loglevel_success : _parameters.loglevel_failure, 
        std::string("request ") + context.AtOr("request", "unknown") + " " + get_request_verb_post() 
        + " with status " + mse::to_string(status.code) + (status.details.empty() ? std::string("") : (std::string(" (") + status.details + ")"))
    );
    return status;
}

std::string LoggingRequestHook::get_request_verb_pre() const
{
    switch (GetRequestType())
    {
    case mse::RequestType::incoming:
        return "handling";
    case mse::RequestType::outgoing:
        return "issuing";
    default:
        return "processing";
    }
}

std::string LoggingRequestHook::get_request_verb_post() const
{
    switch (GetRequestType())
    {
    case mse::RequestType::incoming:
        return "handled";
    case mse::RequestType::outgoing:
        return "issued";
    default:
        return "processed";
    }
}
