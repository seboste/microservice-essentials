#include <core/core.h>
#include <adapters/in-memory-status-db/in-memory-status-db.h>
#include <adapters/http-starwars-client/http-starwars-client.h>
//#include <adapters/dummy-starwars-client/dummy-starwars-client.h>
#include <adapters/http-handler/http-handler.h>
#include <adapters/grpc-handler/grpc-handler.h>
#include <microservice-essentials/cross-cutting-concerns/graceful-shutdown.h>

int main()
{    
    HttpStarWarsClient client;    
    //DummyStarWarsClient client;
    InMemoryStatusDB db;

    Core core(client, db);

    //HttpHandler handler(core);
    GrpcHandler handler(core);


    mse::GracefulShutdownOnSignal gracefulShutdown(mse::Signal::SIG_SHUTDOWN);
    
    handler.Handle();

    return 0;
}
