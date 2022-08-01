#include "http-starwars-client.h"

namespace
{

std::string GetDescription(const std::string& model, const std::string& manufacturer)
{    
    return std::string("A ") 
        + model + " made by"
        + manufacturer;
}

} //end anon NS


HttpStarWarsClient::HttpStarWarsClient(const std::string& URL)
    : _url(URL)
{
    //TODO: establish connection
}

std::vector<StarshipProperties> HttpStarWarsClient::ListStarShipProperties() const
{
    return {};
}

std::optional<StarshipProperties> HttpStarWarsClient::GetStarShipProperties(const std::string& starshipId) const
{
    return {};
}
