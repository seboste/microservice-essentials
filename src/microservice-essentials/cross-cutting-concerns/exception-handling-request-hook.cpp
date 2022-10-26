#include "exception-handling-request-hook.h"

namespace {

std::string extract_exception_details(const std::exception_ptr& exception)
{
    try
    {
        std::rethrow_exception(exception);
    }
    catch(const std::exception& e)
    {
        return e.what();
    }
    catch(...)
    {        
    }
    return "";
}

}

using namespace mse;
using namespace mse::ExceptionHandling;

const std::vector<ExceptionHandlingRequestHook::ExceptionHandlingDefinition> ExceptionHandlingRequestHook::_default_exception_handling_definitions
{ 
    //reasoning for the defaults:
    //- client errors can be forwarded to the client. No logging required.
    //- server errors remain private. Logging is required for diagnosis. Client must know about details.
    { Is<ExceptionOfType<std::invalid_argument>>()  , Status{StatusCode::invalid_argument,  "invalid argument exception"},      mse::LogLevel::invalid, true },
    { Is<ExceptionOfType<std::out_of_range>>()      , Status{StatusCode::out_of_range,      "out of range argument exception"}, mse::LogLevel::invalid, true },
    { Is<AnyException>()                            , Status{StatusCode::internal,          "unknown exception"},               mse::LogLevel::warn,    false }
};

ExceptionHandlingRequestHook::Parameters::Parameters(const std::vector<ExceptionHandlingDefinition>& ehd)
    : exception_handling_definitions(ehd)
{
}

ExceptionHandlingRequestHook::ExceptionHandlingRequestHook(const Parameters& parameters)
    : RequestHook("exception handling")
    , _parameters(parameters)
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
    catch(...)
    {
        //check all exception types that shall be handled
        for(const ExceptionHandlingDefinition& exception_handling_definition : _parameters.exception_handling_definitions)
        {
            if(exception_handling_definition.exception_predicate->Test(std::current_exception()))
            {
                
                if(exception_handling_definition.log_level == mse::LogLevel::invalid
                && !exception_handling_definition.forward_exception_details_to_caller)
                {
                    //exception details are not required => early exit
                    return exception_handling_definition.status;
                }
                
                mse::Status status = exception_handling_definition.status;
                std::string exception_details = extract_exception_details(std::current_exception());
                
                if(exception_handling_definition.log_level != mse::LogLevel::invalid)
                {
                    mse::LogProvider::GetInstance().GetLogger().Write(
                        exception_handling_definition.log_level,
                        std::string("caught exception: ") + exception_details
                        );
                }
                
                if(exception_handling_definition.forward_exception_details_to_caller)
                {
                    if(status.details.empty())
                    {
                        status.details = exception_details;
                    }
                    else
                    {
                        status.details += std::string(": ") + exception_details;
                    }
                }
                
                return status;
            }
        }
        //rethrow 
        throw;
    }
}
