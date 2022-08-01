#pragma once
#include <ports/model.h>
#include <optional>

class StatusDB
{
    public:
        StatusDB() = default;
        virtual ~StatusDB() = default;
        
        virtual std::optional<StarshipStatus> GetStatus(const std::string& starshipId) const = 0;
        virtual void SetStatus(const std::string& starshipId, StarshipStatus status) = 0;
};
