#include <core/core.h>
#include <adapters/in-memory-status-repo/in-memory-status-repo.h>
#include <adapters/http-starwars-client/http-starwars-client.h>

int main()
{
    HttpStarWarsClient client("swapi.dev/api/");
    InMemoryStatusRepo repo;

    Core core(client, repo);


    return 0;
}
