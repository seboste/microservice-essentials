#include "exception-handling-request-hook.h"

using namespace mse;
using namespace mse::ExceptionHandling;

const std::vector<ExceptionHandlingRequestHook::ExceptionToStatusMapping> ExceptionHandlingRequestHook::_default_exception_to_status_mappings
{ 
    { Is<ExceptionOfType<std::invalid_argument>>()  , Status{StatusCode::invalid_argument,  "invalid argument exception"} },
    { Is<ExceptionOfType<std::out_of_range>>()      , Status{StatusCode::out_of_range,      "out of range argument exception"} },
    { Is<AnyException>()                            , Status{StatusCode::internal,          "unknown exception"} }  
};

ExceptionHandlingRequestHook::Parameters::Parameters(const std::vector<ExceptionToStatusMapping>& mappings)
    : exception_to_status_mapping(mappings)
{
}

ExceptionHandlingRequestHook::ExceptionHandlingRequestHook(const Parameters& parameters)
    : RequestHook("exception handling")
    , _exception_to_status_mappings(parameters.exception_to_status_mapping)
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
        for(const ExceptionToStatusMapping& exception_to_status : _exception_to_status_mappings)
        {
            if(exception_to_status.first->Test(std::current_exception()))
            {
                return exception_to_status.second;
            }
        }
        //rethrow 
        throw;
    }
}
