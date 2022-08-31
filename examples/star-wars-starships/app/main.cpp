#include <core/core.h>
#include <adapters/in-memory-status-db/in-memory-status-db.h>
#include <adapters/http-starwars-client/http-starwars-client.h>
//#include <adapters/dummy-starwars-client/dummy-starwars-client.h>
#include <adapters/http-handler/http-handler.h>
#include <adapters/grpc-handler/grpc-handler.h>

int main()
{    
    HttpStarWarsClient client;    
    //DummyStarWarsClient client;
    InMemoryStatusDB db;

    Core core(client, db);

    //HttpHandler handler(core, "0.0.0.0", 8080);
    GrpcHandler handler(core, "0.0.0.0", 50051);
    
    handler.Handle();

    return 0;
}
