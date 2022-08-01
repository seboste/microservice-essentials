#include "in-memory-status-repo.h"

std::optional<StarshipStatus> InMemoryStatusRepo::GetStatus(const std::string& starshipId) const
{
    auto it = _inMemoryDB.find(starshipId);
    if(it == _inMemoryDB.end())
    {
        return {};
    }
    return it->second;
}
    
void InMemoryStatusRepo::SetStatus(const std::string& starshipId, StarshipStatus status)
{
    _inMemoryDB.insert_or_assign(starshipId, status);
}

