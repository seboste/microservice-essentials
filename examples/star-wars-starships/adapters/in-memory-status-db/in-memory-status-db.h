#pragma once

#include <ports/status-db.h>
#include <unordered_map>

class InMemoryStatusDB : public StatusDB
{
public:
  InMemoryStatusDB() = default;
  virtual ~InMemoryStatusDB() = default;

  virtual std::optional<StarshipStatus> GetStatus(const std::string& starshipId) const override;
  virtual void SetStatus(const std::string& starshipId, StarshipStatus status) override;

private:
  std::unordered_map<std::string, StarshipStatus> _inMemoryDB;
};
