#pragma once

#include <ports/starwars-client.h>
#include <string>

class HttpStarWarsClient : public StarWarsClient
{
public:
    HttpStarWarsClient(const std::string& URL);
    virtual ~HttpStarWarsClient() = default;

    virtual std::vector<StarshipProperties> ListStarShipProperties() const override;
    virtual std::optional<StarshipProperties> GetStarShipProperties(const std::string& starshipId) const override;

private:
    std::string _url;
};
