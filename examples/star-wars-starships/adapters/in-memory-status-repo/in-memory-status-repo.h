#pragma once

#include <ports/status-repo.h>
#include <unordered_map>

class InMemoryStatusRepo : public StatusRepo
{
public:
    InMemoryStatusRepo() = default;
    virtual ~InMemoryStatusRepo() = default;
    
    virtual std::optional<StarshipStatus> GetStatus(const std::string& starshipId) const override;
    virtual void SetStatus(const std::string& starshipId, StarshipStatus status) override;

private:
    std::unordered_map<std::string, StarshipStatus> _inMemoryDB;
};
