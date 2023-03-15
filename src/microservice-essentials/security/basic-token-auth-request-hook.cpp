#include "basic-token-auth-request-hook.h"

using namespace mse;


BasicTokenAuthRequestHook::Parameters::Parameters(const std::string& md_key, const std::string& req_token_val)
    : metadata_key(md_key)
    , required_token_value(req_token_val)
{
}

BasicTokenAuthRequestHook::BasicTokenAuthRequestHook(const Parameters& parameters)
    : TokenAuthRequestHook("basic token authentication", parameters.metadata_key, {})
    , _required_token_value(parameters.required_token_value)
{
}

bool BasicTokenAuthRequestHook::decode_token(const std::string& token, std::any& decoded_token, std::string& decoding_details) const
{
    decoding_details = "";
    decoded_token = token;
    return true;
}

bool BasicTokenAuthRequestHook::verify_token(const std::any& decoded_token, std::string& verification_details) const
{
    const std::string token = std::any_cast<std::string>(decoded_token);
    if(token == _required_token_value)
    {
        verification_details = "";
        return true;
    }
    else
    {
        verification_details = "invalid token";
        return false;
    }    
}

std::optional<std::string> BasicTokenAuthRequestHook::extract_claim(const std::any&, const std::string&) const
{
    return std::nullopt;
}

