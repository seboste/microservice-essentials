#include "in-memory-status-db.h"

std::optional<StarshipStatus> InMemoryStatusDB::GetStatus(const std::string& starshipId) const
{
  auto it = _inMemoryDB.find(starshipId);
  if (it == _inMemoryDB.end())
  {
    return {};
  }
  return it->second;
}

void InMemoryStatusDB::SetStatus(const std::string& starshipId, StarshipStatus status)
{
  _inMemoryDB.insert_or_assign(starshipId, status);
}
