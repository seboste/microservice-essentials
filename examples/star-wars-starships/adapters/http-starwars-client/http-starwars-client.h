#pragma once

#include <memory>
#include <ports/starwars-client.h>
#include <string>
#include <vector>

namespace httplib
{
class Client;
}

namespace mse
{
class Cache;
}

class HttpStarWarsClient : public StarWarsClient
{
public:
  HttpStarWarsClient(const std::string& url, // where to reach find the star wars endoints? (e.g. https://swapi.dev)
                     const std::vector<std::string>&
                         headers_to_propagate // which headers shall be propagated from this service to the star wars
                                              // service? (e.g. tracing headers or authorization headers)
  );
  virtual ~HttpStarWarsClient();

  virtual std::vector<StarshipProperties> ListStarShipProperties() const override;
  virtual std::optional<StarshipProperties> GetStarShipProperties(const std::string& starshipId) const override;

private:
  std::unique_ptr<httplib::Client> _cli;
  std::vector<std::string> _headers_to_propagate;
  std::shared_ptr<mse::Cache> _listStarShipsCache;
  std::shared_ptr<mse::Cache> _getStarShipPropertiesCache;
};
