#pragma once

#include <ports/model.h>
#include <string>
#include <vector>

class Api
{
public:
  Api() = default;
  virtual ~Api() = default;

  virtual std::vector<Starship> ListStarShips() const = 0;
  virtual Starship GetStarShip(const std::string& Id) const = 0;
  virtual void UpdateStatus(const std::string& StarShipId, StarshipStatus status) = 0;
};
