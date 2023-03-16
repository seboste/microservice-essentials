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
        typedef std::map<std::string, std::set<std::string>> TokensPerScope;
        Parameters(const std::string& md_key = "authorization", const std::string& req_token_val = "secret-token");
        Parameters(const std::string& md_key, const TokensPerScope& tokensPerScope);

        std::string metadata_key;          
        TokensPerScope valid_tokens; //maps a scope to a set of valid tokens; should not be stored in code
        
        AutoRequestHookParameterRegistration<BasicTokenAuthRequestHook::Parameters, BasicTokenAuthRequestHook> auto_registration;
    };

    BasicTokenAuthRequestHook(const Parameters& parameters);
    virtual ~BasicTokenAuthRequestHook() = default;

protected:

    virtual bool decode_token(const std::string& token, std::any& decoded_token, std::string& decoding_details) const override;
    virtual bool verify_token(const std::any& decoded_token, std::string& verification_details) const override;
    virtual std::optional<std::string> extract_claim(const std::any& decoded_token, const std::string& claim) const override;

private:    

    std::set<std::string> _all_valid_tokens;
    typedef std::map<std::string, std::string> ScopesPerToken; //token -> scopes ; scopes is a list separated by space
    ScopesPerToken _scopes;
};

}
