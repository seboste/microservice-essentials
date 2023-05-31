#pragma once

#include <ports/starwars-client.h>
#include <string>

class DummyStarWarsClient : public StarWarsClient
{
public:
  DummyStarWarsClient();
  virtual ~DummyStarWarsClient() = default;

  virtual std::vector<StarshipProperties> ListStarShipProperties() const override;
  virtual std::optional<StarshipProperties> GetStarShipProperties(const std::string& starshipId) const override;

private:
};
