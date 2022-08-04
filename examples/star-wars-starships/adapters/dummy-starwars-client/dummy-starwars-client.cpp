#include "dummy-starwars-client.h"

namespace
{

const std::vector<StarshipProperties> starships = { { "0", "death star", "this is a death star"}, { "1", "millenium falcon", "this is the millenium falcon"} };

} //end anon NS


DummyStarWarsClient::DummyStarWarsClient()    
{
}

std::vector<StarshipProperties> DummyStarWarsClient::ListStarShipProperties() const
{
    return starships;
}

std::optional<StarshipProperties> DummyStarWarsClient::GetStarShipProperties(const std::string& starshipId) const
{    
    return starships[std::atoi(starshipId.c_str())];
}
