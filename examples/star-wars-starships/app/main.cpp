#include <core/core.h>
#include <adapters/in-memory-status-db/in-memory-status-db.h>
#include <adapters/http-starwars-client/http-starwars-client.h>

int main()
{
    HttpStarWarsClient client("swapi.dev/api/");
    InMemoryStatusDB db;

    Core core(client, db);


    return 0;
}
