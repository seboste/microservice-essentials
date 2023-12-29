#include <adapters/http-starwars-client/http-starwars-client.h>
#include <adapters/in-memory-status-db/in-memory-status-db.h>
#include <core/core.h>
// #include <adapters/dummy-starwars-client/dummy-starwars-client.h>
#include <adapters/grpc-handler/grpc-handler.h>
#include <adapters/http-handler/http-handler.h>
#include <microservice-essentials/cross-cutting-concerns/exception-handling-request-hook.h>
#include <microservice-essentials/cross-cutting-concerns/graceful-shutdown.h>
#include <microservice-essentials/observability/logger.h>
#include <microservice-essentials/observability/logging-request-hook.h>
#include <microservice-essentials/reliability/circuit-breaker-request-hook.h>
#include <microservice-essentials/request/request-processor.h>
#include <microservice-essentials/security/basic-token-auth-request-hook.h>

int main()
{
  mse::Context::GetGlobalContext().Insert(
      {{"app", mse::getenv_or("APP", "star-wars-starships")}, {"version", mse::getenv_or("VERSION", "1.0.0")}});

  mse::ConsoleLogger logger;
  mse::StructuredLogger structured_logger(logger);

  mse::RequestHandler::GloballyWith(mse::LoggingRequestHook::Parameters{});
  mse::RequestHandler::GloballyWith(mse::BasicTokenAuthRequestHook::Parameters(
      "authorization",
      {
          std::make_pair(
              std::string("secret-read-only-token"),
              std::vector<std::string>{"read"}), // this token allows to read (!!token should not be in source code!!)
          std::make_pair(
              std::string("secret-token"),
              std::vector<std::string>{
                  "read", "write"}) // this token allows to read and write (!!token should not be in source code!!)
      }));
  mse::RequestHandler::GloballyWith(mse::ExceptionHandlingRequestHook::Parameters{});

  mse::RequestIssuer::GloballyWith(mse::LoggingRequestHook::Parameters{});
  mse::RequestIssuer::GloballyWith(mse::CircuitBreakerRequestHook::Parameters(
      std::make_shared<mse::MaxPendingRquestsExceededCircuitBreakerStrategy>(2)));

  HttpStarWarsClient client("https://swapi.dev", {}); // nothing to propagate to this external service
  // DummyStarWarsClient client;
  InMemoryStatusDB db;

  Core core(client, db);

  std::unique_ptr<mse::Handler> handler;
  const std::string handlerType = mse::getenv_or("HANDLER_TYPE", "HTTP");
  if (handlerType == "HTTP")
  {
    handler = std::make_unique<HttpHandler>(core);
  }
  else if (handlerType == "GRPC")
  {
    handler = std::make_unique<GrpcHandler>(core);
  }
  else
  {
    throw std::invalid_argument("invalid handler type");
  }

  mse::GracefulShutdownOnSignal gracefulShutdown(mse::Signal::SIG_SHUTDOWN);

  handler->Handle();

  return 0;
}
