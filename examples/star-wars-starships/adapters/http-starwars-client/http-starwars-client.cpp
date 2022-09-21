#include "http-starwars-client.h"
#include <microservice-essentials/context.h>
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
    mse::Context ctx; //TODO: get from handler via threadlocal storage

    std::vector<StarshipProperties> starships;    
    for(std::string path = "/api/starships/?format=json"; path != "";)
    {
        auto resp = _cli->Get(
            path,
            mse::FromContextMetadata<httplib::Headers>(ctx.GetMetadata())
        );
        json data = json::parse(resp->body);
        json nextNode = data.at("next");
        path = nextNode.is_null() ? std::string() : nextNode.get<std::string>();
        for(json starshipNode : data.at("results"))
        {
            starships.emplace_back(from_json(starshipNode));            
        }
    }
    return starships;
}

std::optional<StarshipProperties> HttpStarWarsClient::GetStarShipProperties(const std::string& starshipId) const
{
    mse::Context ctx; //TODO: get from handler via threadlocal storage

    auto resp = _cli->Get(
        std::string("/api/starships/") + starshipId + "/?format=json", 
        mse::FromContextMetadata<httplib::Headers>(ctx.GetMetadata())
        );
    if(!resp || (resp->status != 404 && resp->status != 200))
    {
        throw std::runtime_error("invalid response");
    }

    if(resp->status == 404)
    {
        return std::nullopt;    
    }

    return from_json(json::parse(resp->body));
}
