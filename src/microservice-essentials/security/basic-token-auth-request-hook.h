#pragma once

#include <microservice-essentials/security/token-auth-request-hook.h>
#include <microservice-essentials/request/request-hook-factory.h>
#include <string>

namespace mse
{

class BasicTokenAuthRequestHook : public mse::TokenAuthRequestHook
{
public:
    struct Parameters
    {
        std::string metadata_key = "authorization";
        std::string required_token_value = "secret-token";
        
        AutoRequestHookParameterRegistration<BasicTokenAuthRequestHook::Parameters, BasicTokenAuthRequestHook> auto_registration;
    };

    BasicTokenAuthRequestHook(const Parameters& parameters);
    virtual ~BasicTokenAuthRequestHook() = default;

protected:
    virtual bool is_valid(const std::string& token) const override;

private:
    std::string _required_token_value;
};


}