#pragma

#include <string>
#include <vector>

class StarWarsClient
{
public:
    StarWarsClient() = default;
    virtual ~StarWarsClient() = default;
    
    struct Starship
    {
        std::string Name;
        std::string Model;
        std::string Manufacturer;
        std::string StarshipClass;

        float Length = 0.0f;
        float MaxAtmospheringSpeed = 0.0f;
        float HyperdriveRating = 0.0f;        
        uint64_t Crew = 0;
        uint64_t Passengers = 0;
        float CargoCapacity = 0.0f;
        
        std::vector<std::string> FilmIds;
    };

    struct Film
    {
        std::string Id = 0;
        int Episode = 0;
    };

    virtual std::vector<Starship> ListStarShips() const = 0;
    virtual Film GetFilm(const std::string& id) const = 0;

};