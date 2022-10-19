#pragma once

#include <microservice-essentials/request/request-hook.h>
#include <microservice-essentials/observability/logger.h>


#include <microservice-essentials/request/request-hook-factory.h>

namespace mse
{

template<typename ParametersT, typename RequestHookT>
class AutoRequestHookParameterRegistration
{
    public:
        static std::unique_ptr<RequestHookT> Create(const std::any& parameters)
        {
            return std::make_unique<RequestHookT>(std::any_cast<ParametersT>(parameters));
        }

        AutoRequestHookParameterRegistration()
        {
            static bool registration_done = []()
            {
                mse::RequestHookFactory::GetInstance().Register<ParametersT>(Create);
                return true;
            }();
        }
};

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
