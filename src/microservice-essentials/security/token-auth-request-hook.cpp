#include "token-auth-request-hook.h"


using namespace mse;

TokenAuthRequestHook::TokenAuthRequestHook(const std::string& name, const std::string& token_metadata_key, std::initializer_list<std::string> required_claims)
    : RequestHook(name)
    , _token_metadata_key(token_metadata_key)
    , _required_claims(required_claims.begin(), required_claims.end())
{
}
    
Status TokenAuthRequestHook::pre_process(Context& context)
{
    //security feature: required claims must always come from the token and not from e.g. the request metadata
    //=> make sure that the claim is empty
    for(const auto& claim : _required_claims)
    {
        context.Erase(claim);
        context.Insert(claim, "");
    }

    if(!context.Contains(_token_metadata_key))
    {
        return Status { StatusCode::unauthenticated, std::string("metadata key '") + _token_metadata_key + ("' is missing") };
    }
    const std::string token = context.At(_token_metadata_key);

    std::any decoded_token;
    std::string decoding_details;    
    if(!decode_token(token, decoded_token, decoding_details))
    {
        return Status { StatusCode::unauthenticated, std::string("unable to decode token: ") + decoding_details };
    }

    std::string verification_details;
    if(!verify_token(decoded_token, verification_details))
    {
        return Status { StatusCode::unauthenticated, std::string("token verification failed: ") + verification_details };
    }

    for(const auto& claim : _required_claims)
    {
        context.Erase(claim); //first erase to avoid duplicates in multimap

        std::optional<std::string> claim_value = extract_claim(decoded_token, claim);
        if(!claim_value.has_value())
        {
            return Status { StatusCode::unauthenticated, std::string("required claim not available: ") + claim };
        }
        context.Insert(claim, claim_value.value());
    }

    return Status::OK;
}


