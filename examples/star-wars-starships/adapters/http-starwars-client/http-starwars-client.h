#pragma once

#include <ports/starwars-client.h>
#include <memory>
#include <set>

namespace httplib { class Client; }

class HttpStarWarsClient : public StarWarsClient
{
public:
    HttpStarWarsClient(const std::string& url = "https://swapi.dev", const std::set<std::string>& propagation_header_fields = {}); //propagation_header_fields must be lower case
    virtual ~HttpStarWarsClient();

    virtual std::vector<StarshipProperties> ListStarShipProperties() const override;
    virtual std::optional<StarshipProperties> GetStarShipProperties(const std::string& starshipId) const override;

private:    
    std::unique_ptr<httplib::Client> _cli;
    std::set<std::string> _propagation_header_fields;
};
