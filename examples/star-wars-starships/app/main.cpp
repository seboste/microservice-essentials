#include <core/core.h>
#include <adapters/in-memory-status-db/in-memory-status-db.h>
#include <adapters/http-starwars-client/http-starwars-client.h>
#include <adapters/http-handler/http-handler.h>

int main()
{
    HttpStarWarsClient client("swapi.dev/api/");
    InMemoryStatusDB db;

    Core core(client, db);

    HttpHandler handler(core, "0.0.0.0", 8080);

    handler.Handle();

    return 0;
}
