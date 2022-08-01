#pragma once

#include <ports/api.h>
#include <ports/starwars-client.h>
#include <ports/status-repo.h>

class Core : public Api
{
public:    
    Core(const StarWarsClient& starwarsClient, StatusRepo& statusRepo);
    virtual ~Core() = default;

    virtual std::vector<Starship> ListStarShips() const override;
    virtual Starship GetStarShip(const std::string& Id) const override;
    virtual void UpdateStatus(const std::string& StarShipId, StarshipStatus status) override;
    
private:
    const StarWarsClient& _starwarsClient;
    StatusRepo& _statusRepo;
};
