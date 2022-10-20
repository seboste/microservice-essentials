#pragma once

#include <microservice-essentials/security/token-auth-request-hook.h>
#include <microservice-essentials/request/request-hook-factory.h>
#include <string>

namespace mse
{

/**
 * Request Hook for incoming request that checks if an incoming token in the context's 
 * metadata matches a predefined string.
 * 
 * Beware: This class is just intended for demonstration/testing purposes. Consider implementing
 * a e.g. JWT (https://de.wikipedia.org/wiki/JSON_Web_Token) based mse::TokenAuthRequestHook instead.
*/
class BasicTokenAuthRequestHook : public mse::TokenAuthRequestHook
{
public:
    struct Parameters
    {
        Parameters(const std::string& md_key = "authorization", const std::string& req_token_val = "secret-token");
        std::string metadata_key;
        std::string required_token_value;  //should not be stored in code
        
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
