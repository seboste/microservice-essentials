#include "token-auth-request-hook.h"


using namespace mse;

TokenAuthRequestHook::TokenAuthRequestHook(const std::string& name, const std::string& token_metadata_key)
    : RequestHook(name)
    , _token_metadata_key(token_metadata_key)
{
}
    
Status TokenAuthRequestHook::pre_process(Context& context)
{
    if(!context.Contains(_token_metadata_key))
    {
        return Status { StatusCode::unauthenticated, std::string("metadata key '") + _token_metadata_key + ("' is missing") };
    }

    std::string token = context.At(_token_metadata_key);
    if(!is_valid(token))
    {
        return Status { StatusCode::unauthenticated, "invalid token" };
    }

    return Status::OK;
}


