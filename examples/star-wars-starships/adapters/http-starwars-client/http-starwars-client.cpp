#include "http-starwars-client.h"

namespace
{

std::string GetDescription(const std::string& model, const std::string& manufacturer)
{    
    return std::string("A ") 
        + model + " made by"
        + manufacturer;
}

const std::vector<StarshipProperties> starships = { { "0", "death star", "this is a death star"}, { "1", "millenium falcon", "this is the millenium falcon"} };

} //end anon NS


HttpStarWarsClient::HttpStarWarsClient(const std::string& URL)
    : _url(URL)
{
    //TODO: establish connection
}

std::vector<StarshipProperties> HttpStarWarsClient::ListStarShipProperties() const
{
    return starships;
}

std::optional<StarshipProperties> HttpStarWarsClient::GetStarShipProperties(const std::string& starshipId) const
{    
    return starships[std::atoi(starshipId.c_str())];
}
