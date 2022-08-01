#pragma once

#include <vector>
#include <string>


class Service
{
public:
    Service() = default;
    virtual ~Service() = default;

    struct Starship
    {
        std::string Name;
        std::string Description;
        float Length = 0.0f;
        float HyperdriveRating = 0.0f;
        float MaxAtmospheringSpeed = 0.0f;
        uint64_t Crew = 0;
        uint64_t Passengers = 0;
        float CargoCapacity = 0.0f;
        std::vector<int> Episodes;        
    };

    enum class SortBy
    {
        Name,
        Length,
        HyperdriveRating,
        MaxAtmospheringSpeed,
        Crew,
        Passengers,
        CargoCapacity,
        EpisodeFirstAppearance,
        NoOfAppearances
    };

    virtual std::vector<Starship> ListStarShips(SortBy sorter) const = 0;

};
