#pragma once

#include <microservice-essentials/cross-cutting-concerns/exception-handling-helpers.h>
#include <microservice-essentials/request/request-hook.h>
#include <microservice-essentials/request/request-hook-factory.h>
#include <unordered_map>

namespace mse
{

/**
 * The exception type used by ErrorForwardingRequestHook to forward a status. 
 * The details can be obtained from the base classe's what() method.
*/
class ErrorForwardingException : public std::runtime_error
{
public:
    ErrorForwardingException(const mse::Status& status, const std::string& details);    
    const Status& GetStatus() const { return _status; }    
private:
    Status _status;
};

/**
 * A mapper for the ExceptionHandlingRequestHook that translates an Exception of type ErrorForwardingException
 * into an ExceptionHandling::Definition thereby forwarding an error from the callee to the caller of this service.
 * 
 * If forward_details is enabled, all interal information about the error is returned to the caller (use with caution).
 * Otherwise it will just be logged with the specified log level (or not at all if log level is set to invalid).
*/
class ErrorForwardingExceptionMapper : public mse::ExceptionHandling::Mapper
{
    public:
        ErrorForwardingExceptionMapper(mse::LogLevel log_level, bool forward_details);
        virtual std::optional<mse::ExceptionHandling::Definition> Map(const std::exception_ptr& exception) const override;

    private:
        mse::LogLevel _log_level = mse::LogLevel::invalid;
        bool _forward_details = false;
};

/**
 * Request hook for outgoing requests that throws exceptions of type ErrorForwardingException for defined
 * status codes returned by the callee.
 * In the surrounding incoming request, an ErrorForwardingExceptionMapper in the ExceptionHandlingRequestHook
 * is used to catch the exception and forward the defined status to the caller.
 * 
 * By default, StatusCode::unavailable is returned when a callee returns an error.
 * 
 * Note that a ExceptionHandlingRequestHook containing a ErrorForwardingExceptionMapper needs to be used for incoming requests. 
 * The default ExceptionHandlingRequestHook fulfills this critera.
*/
class ErrorForwardingRequestHook : public RequestHook
{
public:

    struct Parameters
    {
        //Builder pattern like methods for convinient creation
        Parameters& IncludeAllErrorCodes(const mse::Status& map_to = mse::Status{ mse::StatusCode::unavailable, "" });
        Parameters& Include(mse::StatusCode map_from, const mse::Status& map_to = mse::Status{ mse::StatusCode::unavailable, "" });
        Parameters& Exclude(mse::StatusCode map_from);

        std::unordered_map<StatusCode, Status> status_code_mapping; //when callee returns status code (.first), the specified status (.second) shall be returned to the caller.
        AutoRequestHookParameterRegistration<ErrorForwardingRequestHook::Parameters, ErrorForwardingRequestHook> auto_registration;
    };
    
    ErrorForwardingRequestHook(const Parameters& parameters);
    virtual ~ErrorForwardingRequestHook();

    virtual Status post_process(Context& context, Status status) override;

private:
    Parameters _parameters;
};

}
