#pragma once

#include <microservice-essentials/request/request-hook.h>
#include <microservice-essentials/observability/logger.h>
#include <microservice-essentials/request/request-hook-factory.h>

namespace mse
{

/**

 */
class LoggingRequestHook : public mse::RequestHook
{    
public:
    struct Parameters
    {
        mse::LogLevel loglevel_success = mse::LogLevel::trace;
        mse::LogLevel loglevel_failure = mse::LogLevel::trace;

        AutoRequestHookParameterRegistration<LoggingRequestHook::Parameters, LoggingRequestHook> auto_registration;
    };

    LoggingRequestHook(const Parameters& parameters = Parameters{ mse::LogLevel::trace, mse::LogLevel::trace});
    virtual ~LoggingRequestHook() = default;

protected:
    virtual Status pre_process(Context& context) override;
    virtual Status post_process(Context& context, Status status) override;    

private:
    std::string get_request_verb_pre() const;
    std::string get_request_verb_post() const;

    Parameters _parameters;
};

}
