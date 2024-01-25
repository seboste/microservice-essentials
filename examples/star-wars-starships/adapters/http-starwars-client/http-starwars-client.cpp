#include "http-starwars-client.h"
#include <microservice-essentials/context.h>
#include <microservice-essentials/cross-cutting-concerns/error-forwarding-request-hook.h>
#include <microservice-essentials/observability/logger.h>
#include <microservice-essentials/performance/caching-request-hook.h>
#include <microservice-essentials/reliability/retry-request-hook.h>
#include <microservice-essentials/request/request-processor.h>
#include <microservice-essentials/utilities/metadata-converter.h>
#include <microservice-essentials/utilities/status-converter.h>
#define CPPHTTPLIB_OPENSSL_SUPPORT // be consistent with other projects to prevent seg fault
#include <chrono>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <regex>

using namespace std::literals::chrono_literals;
using json = nlohmann::json;

namespace
{

std::string getDescription(const std::string& model, const std::string& manufacturer)
{
  return std::string("A ") + model + " made by " + manufacturer;
}

std::string extractId(const std::string& path)
{
  std::smatch ip_result;
  std::regex_match(path, ip_result, std::regex("(.*)/api/starships/(.*)/"));
  if (ip_result.size() != 3)
  {
    throw std::invalid_argument("invalid URL");
  }
  return ip_result[2];
}

StarshipProperties from_json(const json& node)
{
  return {extractId(node.at("url").get<std::string>()), node.at("name").get<std::string>(),
          getDescription(node.at("model").get<std::string>(), node.at("manufacturer").get<std::string>())};
}

} // namespace

HttpStarWarsClient::HttpStarWarsClient(const std::string& url, const std::vector<std::string>& headers_to_propagate)
    : _cli(std::make_unique<httplib::Client>(url)), _headers_to_propagate(headers_to_propagate),
      _cache(std::make_shared<mse::LRUCache>(std::make_shared<mse::UnorderedMapCache>(),
                                             5)) // LRU cache with capacity for 5 entries
{
}

HttpStarWarsClient::~HttpStarWarsClient()
{
}

std::vector<StarshipProperties> HttpStarWarsClient::ListStarShipProperties() const
{
  std::vector<StarshipProperties> starships;

  mse::RequestIssuer("ListStarShipProperties", mse::Context())
      .BeginWith(mse::ErrorForwardingRequestHook::Parameters().IncludeAllErrorCodes())
      .With(mse::RetryRequestHook::Parameters(std::make_shared<mse::BackoffGaussianJitterDecorator>(
          std::make_shared<mse::LinearRetryBackoff>(3, 10000ms), 1000ms)))
      .Process([&](mse::Context& context) {
        for (std::string path = "/api/starships/?format=json"; path != "";)
        {
          auto resp = _cli->Get(
              path, mse::FromContextMetadata<httplib::Headers>(context.GetFilteredMetadata(_headers_to_propagate)));
          if (!resp)
          {
            return mse::Status{mse::StatusCode::unknown, ""};
          }
          if (mse::Status status{mse::FromHttpStatusCode(resp->status), ""}; !status)
          {
            return status;
          }

          json data = json::parse(resp->body);
          json nextNode = data.at("next");
          path = nextNode.is_null() ? std::string() : nextNode.get<std::string>();
          for (json starshipNode : data.at("results"))
          {
            starships.emplace_back(from_json(starshipNode));
          }
        }
        return mse::Status::OK;
      });

  return starships;
}

std::optional<StarshipProperties> HttpStarWarsClient::GetStarShipProperties(const std::string& starshipId) const
{
  std::optional<StarshipProperties> starshipProperties = std::nullopt;

  mse::RequestIssuer("GetStarShipProperties", mse::Context())
      .BeginWith(
          mse::ErrorForwardingRequestHook::Parameters().IncludeAllErrorCodes().Exclude(mse::StatusCode::not_found))
      .With(mse::CachingRequestHook::Parameters(_cache)
                .WithKey(starshipId)
                .WithCachedObject(starshipProperties)
                .NeverExpire()
                .Include(mse::StatusCode::not_found))
      .With(mse::RetryRequestHook::Parameters(std::make_shared<mse::BackoffGaussianJitterDecorator>(
          std::make_shared<mse::LinearRetryBackoff>(3, 10000ms), 1000ms)))
      .Process([&](mse::Context&) {
        mse::Status status{mse::StatusCode::unknown, ""};
        mse::Context client_context = mse::Context::GetThreadLocalContext();
        if (auto resp = _cli->Get(
                std::string("/api/starships/") + starshipId + "/?format=json",
                mse::FromContextMetadata<httplib::Headers>(client_context.GetFilteredMetadata(_headers_to_propagate)));
            resp)
        {
          status.code = mse::FromHttpStatusCode(resp->status);
          if (status)
          {
            starshipProperties = from_json(json::parse(resp->body));
          }
        }
        return status;
      });
  return starshipProperties;
}
