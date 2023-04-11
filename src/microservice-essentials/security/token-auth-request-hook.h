#pragma once

#include <microservice-essentials/request/request-hook.h>
#include <any>
#include <initializer_list>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace mse
{

/**
 * Abstract Request Hook for incoming request that extracts a token from 
 * a predefined metadata key and passes it to its subclass for decoding,
 * verification, and scope claim extraction before the request can be further 
 * processed.
 * Returns StatusCode::unauthenticated if the token is not present, it cannot
 * be decoded, the verification fails, or not all required claims are present.
*/
class TokenAuthRequestHook : public mse::RequestHook
{
public:    
    TokenAuthRequestHook(const std::string& name, const std::string& token_metadata_key, const std::vector<std::string>& required_claims);
    virtual ~TokenAuthRequestHook() = default;

protected:

    virtual bool decode_token(const std::string& token, std::any& decoded_token, std::string& decoding_details) const = 0;
    virtual bool verify_token(const std::any& decoded_token, std::string& verification_details) const = 0;
    virtual std::optional<std::string> extract_claim(const std::any& decoded_token, const std::string& claim) const = 0;
    
private:
    virtual Status pre_process(Context& context) override final;

    std::string _token_metadata_key;
    std::vector<std::string> _required_claims;
};

}