#pragma once

#include <ports/service.h>
#include <ports/starwars-client.h>

class Core : public Service
{
public:
    Core(const StarWarsClient& starwarsClient);
    virtual ~Core() = default;

    virtual std::vector<Starship> ListStarShips(SortBy sorter) const override;
private:
    const StarWarsClient& _starwarsClient;
};
