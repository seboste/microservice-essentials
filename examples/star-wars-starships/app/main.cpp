#include <core/core.h>
#include <adapters/in-memory-status-db/in-memory-status-db.h>
#include <adapters/http-starwars-client/http-starwars-client.h>
//#include <adapters/dummy-starwars-client/dummy-starwars-client.h>
#include <adapters/http-handler/http-handler.h>
#include <adapters/grpc-handler/grpc-handler.h>
#include <microservice-essentials/cross-cutting-concerns/graceful-shutdown.h>
#include <microservice-essentials/cross-cutting-concerns/exception-handling-request-hook.h>
#include <microservice-essentials/observability/logger.h>
#include <microservice-essentials/observability/logging-request-hook.h>
#include <microservice-essentials/request/request-processor.h>
#include <microservice-essentials/security/basic-token-auth-request-hook.h>

int main()
{
    mse::Context::GetGlobalContext().Insert({ 
            {"app", mse::getenv_or("APP", "star-wars-starships") },
            {"version", mse::getenv_or("VERSION", "1.0.0") }
        });

    mse::ConsoleLogger logger;
    mse::StructuredLogger structured_logger(logger);

    mse::RequestHandler::GloballyWith(mse::LoggingRequestHook::Parameters{});    
    mse::RequestHandler::GloballyWith(mse::BasicTokenAuthRequestHook::Parameters{});
    mse::RequestHandler::GloballyWith(mse::ExceptionHandlingRequestHook::Parameters{});
    
    mse::RequestIssuer::GloballyWith(mse::LoggingRequestHook::Parameters{});

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
