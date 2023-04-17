#pragma once

#include <ports/starwars-client.h>
#include <memory>

namespace httplib { class Client; }

class HttpStarWarsClient : public StarWarsClient
{
public:
    HttpStarWarsClient(const std::string& url = "https://swapi.dev");
    virtual ~HttpStarWarsClient();

    virtual std::vector<StarshipProperties> ListStarShipProperties() const override;
    virtual std::optional<StarshipProperties> GetStarShipProperties(const std::string& starshipId) const override;

private:    
    std::unique_ptr<httplib::Client> _cli;    
};
