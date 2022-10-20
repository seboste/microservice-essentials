#include "basic-token-auth-request-hook.h"

using namespace mse;


BasicTokenAuthRequestHook::Parameters::Parameters(const std::string& md_key, const std::string& req_token_val)
    : metadata_key(md_key)
    , required_token_value(req_token_val)
{
}

BasicTokenAuthRequestHook::BasicTokenAuthRequestHook(const Parameters& parameters)
    : TokenAuthRequestHook("basic token authentication", parameters.metadata_key)
    , _required_token_value(parameters.required_token_value)
{
}

bool BasicTokenAuthRequestHook::is_valid(const std::string& token) const
{
    return token == _required_token_value;
}
