#pragma once

#include <optional>
#include <ports/model.h>
#include <string>
#include <vector>

class StarWarsClient
{
public:
  StarWarsClient() = default;
  virtual ~StarWarsClient() = default;

  virtual std::vector<StarshipProperties> ListStarShipProperties() const = 0;
  virtual std::optional<StarshipProperties> GetStarShipProperties(const std::string& starshipId) const = 0;
};
