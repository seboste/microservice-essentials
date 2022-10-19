#include "basic-token-auth-request-hook.h"

using namespace mse;

BasicTokenAuthRequestHook::BasicTokenAuthRequestHook(const Parameters& parameters)
    : TokenAuthRequestHook("basic token authorization", parameters.metadata_key)
    , _required_token_value(parameters.required_token_value)
{
}

bool BasicTokenAuthRequestHook::is_valid(const std::string& token) const
{
    return token == _required_token_value;
}
