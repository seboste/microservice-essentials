#include "core.h"
#include <algorithm>

namespace
{

std::string GetDescription(const StarWarsClient::Starship& clientStarShip)
{    
    return std::string("A ") 
        + clientStarShip.Model + " made by"
        + clientStarShip.Manufacturer;
}

std::vector<int> getEpisodes(const std::vector<std::string>& filmIds, const StarWarsClient& swClient)
{    
    std::vector<int> episodes;
    std::transform(begin(filmIds), end(filmIds), std::back_inserter(episodes),
        [&swClient](const std::string& id)->int { return swClient.GetFilm(id).Episode; }
    );
    return episodes;
}

} //end anon NS

Core::Core(const StarWarsClient& starwarsClient)
    : _starwarsClient(starwarsClient)
{
}

 std::vector<Core::Starship> Core::ListStarShips(SortBy sorter) const
{
    std::vector<Service::Starship> starShips;

    for(const StarWarsClient::Starship& clientStarShip : _starwarsClient.ListStarShips())
    {
        Core::Starship starShip
        {
            clientStarShip.Name,
            GetDescription(clientStarShip),
            clientStarShip.Length,
            clientStarShip.HyperdriveRating,
            clientStarShip.MaxAtmospheringSpeed,
            clientStarShip.Crew,
            clientStarShip.Passengers,
            clientStarShip.CargoCapacity,
            getEpisodes(clientStarShip.FilmIds, _starwarsClient)        
        };

        //TODO: validation        

        starShips.emplace_back(starShip);
    }

    return starShips;
}



