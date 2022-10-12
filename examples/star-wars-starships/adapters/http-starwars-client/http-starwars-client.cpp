#include "http-starwars-client.h"
#include <microservice-essentials/context.h>
#include <microservice-essentials/observability/logger.h>
#include <microservice-essentials/request/request-processor.h>
#include <microservice-essentials/utilities/metadata-converter.h>
#define CPPHTTPLIB_OPENSSL_SUPPORT  //be consistent with other projects to prevent seg fault
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <regex>

using json = nlohmann::json;

namespace
{

std::string getDescription(const std::string& model, const std::string& manufacturer)
{    
    return std::string("A ") 
        + model + " made by "
        + manufacturer;
}

std::string extractId(const std::string& path)
{
    std::smatch ip_result;
    std::regex_match(path, ip_result, std::regex("(.*)/api/starships/(.*)/"));    
    if(ip_result.size() != 3)
    {        
        throw std::invalid_argument("invalid URL");
    }    
    return ip_result[2];
}

StarshipProperties from_json(const json& node)
{
    return {
        extractId(node.at("url").get<std::string>()),
        node.at("name").get<std::string>(),
        getDescription(
            node.at("model").get<std::string>(), 
            node.at("manufacturer").get<std::string>()
        )
    };
}

} //end anon NS


HttpStarWarsClient::HttpStarWarsClient(const std::string& url)
    : _cli(std::make_unique<httplib::Client>(url))
{
}

HttpStarWarsClient::~HttpStarWarsClient()
{
}

std::vector<StarshipProperties> HttpStarWarsClient::ListStarShipProperties() const
{
    std::vector<StarshipProperties> starships;

    mse::RequestIssuer("ListStarShipProperties", mse::Context())
        .Process([&](mse::Context& context)
        {
            MSE_LOG_TRACE("listing starships");
            for(std::string path = "/api/starships/?format=json"; path != "";)
            {
                auto resp = _cli->Get(
                    path,
                    mse::FromContextMetadata<httplib::Headers>(context.GetMetadata())
                );
                json data = json::parse(resp->body);
                json nextNode = data.at("next");
                path = nextNode.is_null() ? std::string() : nextNode.get<std::string>();
                for(json starshipNode : data.at("results"))
                {
                    starships.emplace_back(from_json(starshipNode));            
                }
            }
            return mse::Status();
        });

    return starships;
}

std::optional<StarshipProperties> HttpStarWarsClient::GetStarShipProperties(const std::string& starshipId) const
{
    std::optional<StarshipProperties> starshipProperties = std::nullopt;
    
    mse::Status status = mse::RequestIssuer("GetStarShipProperties", mse::Context())
        .Process([&](mse::Context& context)
        {
            MSE_LOG_TRACE("getting starships");

            mse::Context client_context = mse::Context::GetThreadLocalContext();
            auto resp = _cli->Get(
                std::string("/api/starships/") + starshipId + "/?format=json", 
                mse::FromContextMetadata<httplib::Headers>(client_context.GetMetadata())
                );
            if(!resp || (resp->status != 404 && resp->status != 200))
            {
                return mse::Status({ mse::StatusCode::internal, "invalid response"});                
            }            
            
            if(resp->status != 404)
            {
                starshipProperties = from_json(json::parse(resp->body));
            }
            return mse::Status();
        });

    if(!status)
    {
        throw std::runtime_error("invalid response");
    }

    return starshipProperties;
}
