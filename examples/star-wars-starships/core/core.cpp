#include "core.h"
#include <algorithm>
#include <stdexcept>
#include <iterator>

Core::Core(const StarWarsClient& starwarsClient, StatusDB& statusDB)
    : _starwarsClient(starwarsClient)
    , _statusDB(statusDB)
{
}

std::vector<Starship> Core::ListStarShips() const
{
    std::vector<Starship> starships;
    std::vector<StarshipProperties> properties = _starwarsClient.ListStarShipProperties();
    std::transform(begin(properties), end(properties), std::back_inserter(starships),
        [&](const StarshipProperties& properties) 
        {
            return Starship { 
                properties, 
                _statusDB.GetStatus(properties.Id).value_or(StarshipStatus::Unknown)
            };
        }
    );
    return starships;
}

Starship Core::GetStarShip(const std::string& Id) const
{
    try
    {
        return Starship { 
                    _starwarsClient.GetStarShipProperties(Id).value(),
                    _statusDB.GetStatus(Id).value_or(StarshipStatus::Unknown)
                };
    }
    catch(const std::bad_optional_access& ex) //convert to out of range => 404
    {
        throw std::out_of_range("invalid ship id");
    }
}

void Core::UpdateStatus(const std::string& StarShipId, StarshipStatus status)
{
    if(!_starwarsClient.GetStarShipProperties(StarShipId).has_value())
    {
        throw std::out_of_range("invalid ship id");
    }

    _statusDB.SetStatus(StarShipId, status);
}
