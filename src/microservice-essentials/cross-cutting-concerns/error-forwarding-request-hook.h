#pragma once

#include <microservice-essentials/cross-cutting-concerns/exception-handling-helpers.h>
#include <microservice-essentials/request/request-hook.h>
#include <microservice-essentials/request/request-hook-factory.h>
#include <unordered_map>

namespace mse
{

class ErrorForwardingException : public std::runtime_error
{
public:
    ErrorForwardingException(const mse::Status& status, const std::string& details);    
    const Status& GetStatus() const { return _status; }    
private:
    Status _status;
};

class ErrorForwardingExceptionMapper : public mse::ExceptionHandling::Mapper
{
    public:
        ErrorForwardingExceptionMapper(mse::LogLevel loglevel, bool forward_details);
        virtual std::optional<mse::ExceptionHandling::Definition> Map(const std::exception_ptr& exception) const override;

    private:
        mse::LogLevel _loglevel = mse::LogLevel::invalid;
        bool _forward_details = false;
};

class ErrorForwardingRequestHook : public RequestHook
{
public:

    struct Parameters
    {
        //Builder pattern like methods for convinient creation
        Parameters& IncludeAllErrorCodes(const mse::Status& map_to = mse::Status{ mse::StatusCode::unavailable, "" });
        Parameters& Include(mse::StatusCode map_from, const mse::Status& map_to = mse::Status{ mse::StatusCode::unavailable, "" });
        Parameters& Exclude(mse::StatusCode map_from);

        std::unordered_map<StatusCode, Status> status_code_mapping;
        AutoRequestHookParameterRegistration<ErrorForwardingRequestHook::Parameters, ErrorForwardingRequestHook> auto_registration;
    };
    
    ErrorForwardingRequestHook(const Parameters& parameters);
    virtual ~ErrorForwardingRequestHook();

    virtual Status post_process(Context& context, Status status) override;

private:
    Parameters _parameters;
};

}
