#include "basic-token-auth-request-hook.h"
#include <iterator>
#include <sstream>
#include <string>

using namespace mse;

namespace
{

struct TokenData
{
  std::string token;
  std::string scopes; // space separated string of scopess
};

std::string to_separated_string(const std::vector<std::string>& data, const std::string& delim)
{
  if (data.empty())
  {
    return std::string();
  }

  std::ostringstream stream;
  std::copy(data.begin(), --data.end(), std::ostream_iterator<std::string>(stream, delim.c_str()));
  stream << *data.rbegin();

  return stream.str();
}

} // namespace

BasicTokenAuthRequestHook::Parameters::Parameters(const std::string& md_key, const std::string& req_token_val)
    : metadata_key(md_key), valid_tokens_with_scope({std::make_pair(req_token_val, std::vector<std::string>())})
{
}

BasicTokenAuthRequestHook::Parameters::Parameters(const std::string& md_key, const TokensWithScope& tokens_with_scope)
    : metadata_key(md_key), valid_tokens_with_scope(tokens_with_scope)
{
}

BasicTokenAuthRequestHook::BasicTokenAuthRequestHook(const Parameters& parameters)
    : TokenAuthRequestHook("basic token authentication", parameters.metadata_key, {"scope"})
{
  for (const auto& token_with_scope : parameters.valid_tokens_with_scope)
  {
    _all_valid_tokens.insert(token_with_scope.first);
    _scopes[token_with_scope.first] = to_separated_string(token_with_scope.second, " ");
  }
}

bool BasicTokenAuthRequestHook::decode_token(const std::string& token, std::any& decoded_token,
                                             std::string& decoding_details) const
{
  decoding_details = "";

  std::string scopes;
  if (const auto& cit = _scopes.find(token); cit != _scopes.end())
  {
    scopes = cit->second;
  }

  decoded_token = TokenData{token, scopes};
  return true;
}

bool BasicTokenAuthRequestHook::verify_token(const std::any& decoded_token, std::string& verification_details) const
{
  const TokenData token_data = std::any_cast<TokenData>(decoded_token);
  if (_all_valid_tokens.find(token_data.token) != _all_valid_tokens.end())
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

std::optional<std::string> BasicTokenAuthRequestHook::extract_claim(const std::any& decoded_token,
                                                                    const std::string& claim) const
{
  // only scope is supported
  if (claim == "scope")
  {
    const TokenData token_data = std::any_cast<TokenData>(decoded_token);
    return token_data.scopes;
  }

  return std::nullopt;
}
