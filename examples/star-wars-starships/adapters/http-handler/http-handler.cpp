#include "http-handler.h"
#include <microservice-essentials/context.h>
#include <microservice-essentials/observability/logger.h>
#include <microservice-essentials/performance/caching-request-hook.h>
#include <microservice-essentials/request/request-processor.h>
#include <microservice-essentials/security/claim-checker-request-hook.h>
#include <microservice-essentials/utilities/metadata-converter.h>
#include <microservice-essentials/utilities/status-converter.h>
#define CPPHTTPLIB_OPENSSL_SUPPORT // be consistent with other projects to prevent seg fault
#include <functional>
#include <httplib/httplib.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <regex>

namespace
{

using json = nlohmann::json;

std::string to_string(StarshipStatus status)
{
  switch (status)
  {
  case StarshipStatus::Unknown:
    return "Unknown";
  case StarshipStatus::UnderConstruction:
    return "UnderConstruction";
  case StarshipStatus::OnStandby:
    return "OnStandby";
  case StarshipStatus::InAction:
    return "InAction";
  case StarshipStatus::Damaged:
    return "Damaged";
  case StarshipStatus::Destroyed:
    return "Destroyed";
  }
  throw std::logic_error("invalid status: " + std::to_string(static_cast<int>(status)));
}

StarshipStatus from_string(const std::string& statusString)
{
  static const std::unordered_map<std::string, StarshipStatus> lookupMap = {
      {"Unknown", StarshipStatus::Unknown},     {"UnderConstruction", StarshipStatus::UnderConstruction},
      {"OnStandby", StarshipStatus::OnStandby}, {"InAction", StarshipStatus::InAction},
      {"Damaged", StarshipStatus::Damaged},     {"Destroyed", StarshipStatus::Destroyed},
  };

  const auto cit = lookupMap.find(statusString);
  if (cit == lookupMap.end())
  {
    throw std::logic_error("invalid status: " + statusString);
  }
  return cit->second;
}

json to_json(const Starship& starship)
{
  return json{{"properties",
               {{"id", starship.Properties.Id},
                {"name", starship.Properties.Name},
                {"description", starship.Properties.Description}}},
              {"status", to_string(starship.Status)}};
}

json to_json(const std::vector<Starship>& starships)
{
  json jsonStarShips = json::array();
  for (const Starship& starship : starships)
  {
    jsonStarShips.emplace_back(to_json(starship));
  }
  return jsonStarShips;
}

std::string extractId(const std::string& path)
{
  std::smatch ip_result;
  std::regex_match(path, ip_result, std::regex("/(.*)/(.*)"));
  if (ip_result.size() != 3)
  {
    throw std::invalid_argument("invalid URL");
  }

  return ip_result[2];
}

} // namespace

HttpHandler::HttpHandler(Api& api, const std::string& host, int port)
    : _api(api), _svr(std::make_unique<httplib::Server>()), _host(host), _port(port),
      _cache(std::make_shared<mse::UnorderedMapCache>())
{
  _svr->Get("/StarShips", std::bind(&HttpHandler::listStarShips, this, std::placeholders::_1, std::placeholders::_2));
  _svr->Get("/StarShip/(.*)", std::bind(&HttpHandler::getStarShip, this, std::placeholders::_1, std::placeholders::_2));
  _svr->Put("/StarShipStatus/(.*)", httplib::Server::Handler(std::bind(&HttpHandler::updateStatus, this,
                                                                       std::placeholders::_1, std::placeholders::_2)));
}

HttpHandler::~HttpHandler()
{
}

void HttpHandler::Handle()
{
  MSE_LOG_INFO(std::string("serving on ") + _host + ':' + std::to_string(_port));
  _svr->listen(_host, _port);
}

void HttpHandler::Stop()
{
  MSE_LOG_INFO("stop requested");
  _svr->stop();
}

void HttpHandler::listStarShips(const httplib::Request& request, httplib::Response& response)
{
  std::string content;
  response.status =
      mse::ToHttpStatusCode(mse::RequestHandler("listStarShips", mse::Context(mse::ToContextMetadata(request.headers)))
                                .With(mse::ClaimCheckerRequestHook::ScopeContains("read"))
                                .With(mse::CachingRequestHook::Parameters(_cache)
                                          .WithConstantResponse()
                                          .WithCacheReader([&response](const std::any& data) {
                                            response.set_content(std::any_cast<std::string>(data), "text/json");
                                          })
                                          .WithCacheWriter([&content]() -> std::any { return content; })
                                          .NeverExpire())
                                .Process([&](mse::Context&) {
                                  content = to_json(_api.ListStarShips()).dump();
                                  response.set_content(content, "text/json");
                                  return mse::Status::OK;
                                })
                                .code);
}

void HttpHandler::getStarShip(const httplib::Request& request, httplib::Response& response)
{
  response.status = mse::ToHttpStatusCode(
      mse::RequestHandler("getStarShip", mse::Context(mse::ToContextMetadata(request.headers)))
          .With(mse::ClaimCheckerRequestHook::ScopeContains("read"))
          .Process([&](mse::Context&) {
            response.set_content(to_json(_api.GetStarShip(extractId(request.path))).dump(), "text/json");
            return mse::Status();
          })
          .code);
}

void HttpHandler::updateStatus(const httplib::Request& request, httplib::Response& response)
{
  response.status = mse::ToHttpStatusCode(
      mse::RequestHandler("updateStatus", mse::Context(mse::ToContextMetadata(request.headers)))
          .With(mse::ClaimCheckerRequestHook::ScopeContains("write"))
          .Process([&](mse::Context&) {
            _api.UpdateStatus(extractId(request.path), from_string(json::parse(request.body).at("status")));
            return mse::Status();
          })
          .code);
}
