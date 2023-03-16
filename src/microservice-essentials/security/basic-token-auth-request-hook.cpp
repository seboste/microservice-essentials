#include "basic-token-auth-request-hook.h"
#include <sstream>
#include <string>
#include <iterator>

using namespace mse;

namespace {

struct TokenData
{
    std::string token;
    std::string scopes; //space separated string of scopess
};

std::string to_separated_string(const std::set<std::string> set, const std::string& delim)
{
    if(set.empty())
    {
        return std::string();
    }

    std::ostringstream stream;
    std::copy(set.begin(), --set.end(), std::ostream_iterator<std::string>(stream, delim.c_str()));
    stream << *set.rbegin();

    return stream.str();
}

}

BasicTokenAuthRequestHook::Parameters::Parameters(const std::string& md_key, const std::string& req_token_val)
    : metadata_key(md_key)
    , valid_tokens({std::make_pair(std::string(), std::set<std::string>({ req_token_val }))})
{
}

BasicTokenAuthRequestHook::Parameters::Parameters(const std::string& md_key, const TokensPerScope& tokensPerScope)
    : metadata_key(md_key)
    , valid_tokens(tokensPerScope)
{
}

BasicTokenAuthRequestHook::BasicTokenAuthRequestHook(const Parameters& parameters)
    : TokenAuthRequestHook("basic token authentication", parameters.metadata_key, {})    
{

    typedef std::map<std::string, std::set<std::string>> ScopeSetPerToken;
    ScopeSetPerToken scopeSets;

    for(const auto& tokensForScope : parameters.valid_tokens)
    {        
        _all_valid_tokens.insert(tokensForScope.second.begin(), tokensForScope.second.end());
        scopeSets[tokensForScope.first].insert(tokensForScope.second.begin(), tokensForScope.second.end());        
    }

    for(const auto& scopeSet : scopeSets)
    {
        _scopes[scopeSet.first] = to_separated_string(scopeSet.second, " ");
    }
}

bool BasicTokenAuthRequestHook::decode_token(const std::string& token, std::any& decoded_token, std::string& decoding_details) const
{
    decoding_details = "";
    
    std::string scopes;
    if(const auto& cit = _scopes.find(token); cit!=_scopes.end())
    {
        scopes = cit->second;
    }

    decoded_token = TokenData{token, scopes};
    return true;
}

bool BasicTokenAuthRequestHook::verify_token(const std::any& decoded_token, std::string& verification_details) const
{
    const TokenData token_data = std::any_cast<TokenData>(decoded_token);
    if(_all_valid_tokens.find(token_data.token) != _all_valid_tokens.end())
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

std::optional<std::string> BasicTokenAuthRequestHook::extract_claim(const std::any& decoded_token, const std::string& claim) const
{
    //only scope is supported 
    if(claim == "scope")
    {
        const TokenData token_data = std::any_cast<TokenData>(decoded_token);    
        return token_data.scopes;        
    }

    return std::nullopt;    
}


