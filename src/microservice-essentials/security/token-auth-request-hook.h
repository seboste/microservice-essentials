#pragma once

#include <microservice-essentials/request/request-hook.h>

namespace mse
{

class TokenAuthRequestHook : public mse::RequestHook
{
public:    
    TokenAuthRequestHook(const std::string& name, const std::string& token_metadata_key);
    virtual ~TokenAuthRequestHook() = default;

protected:
    virtual bool is_valid(const std::string& token) const = 0;

private:
    virtual Status pre_process(Context& context) override final;

    std::string _token_metadata_key;
};

}