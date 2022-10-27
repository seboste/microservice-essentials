#pragma once

#include <microservice-essentials/request/request-hook.h>
#include <microservice-essentials/request/request-hook-factory.h>
#include <unordered_map>

namespace mse
{

class ErrorForwardingException : public std::runtime_error
{
public:
    ErrorForwardingException(const mse::Status& status);

    const Status& GetStatus() const { return _status; }

private:
    Status _status;
};


class ErrorForwardingRequestHook : public RequestHook
{
public:

    struct Parameters
    {
        std::unordered_map<StatusCode, StatusCode> status_code_mapping;
        AutoRequestHookParameterRegistration<ErrorForwardingRequestHook::Parameters, ErrorForwardingRequestHook> auto_registration;
    };

    ErrorForwardingRequestHook(const Parameters& parameters);
    virtual ~ErrorForwardingRequestHook();

    virtual Status post_process(Context& context, Status status) override;

private:
    Parameters _parameters;
};

}
