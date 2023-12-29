#include "grpc-handler.h"
#include <generated/api.grpc.pb.h>
#include <grpcpp/grpcpp.h>
#include <microservice-essentials/context.h>
#include <microservice-essentials/handler.h>
#include <microservice-essentials/observability/logger.h>
#include <microservice-essentials/performance/caching-request-hook.h>
#include <microservice-essentials/request/request-processor.h>
#include <microservice-essentials/security/claim-checker-request-hook.h>
#include <microservice-essentials/utilities/metadata-converter.h>
#include <microservice-essentials/utilities/status-converter.h>

using namespace grpc;

namespace
{

StarShips::StarshipStatus to_protobuf(StarshipStatus status)
{
  switch (status)
  {
  case StarshipStatus::Unknown:
    return StarShips::StarshipStatus::STARSHIP_STATUS_UNKNOWN;
  case StarshipStatus::UnderConstruction:
    return StarShips::StarshipStatus::STARSHIP_STATUS_UNDER_CONSTRUCTION;
  case StarshipStatus::OnStandby:
    return StarShips::StarshipStatus::STARSHIP_STATUS_ON_STANDBY;
  case StarshipStatus::InAction:
    return StarShips::StarshipStatus::STARSHIP_STATUS_IN_ACTION;
  case StarshipStatus::Damaged:
    return StarShips::StarshipStatus::STARSHIP_STATUS_DAMAGED;
  case StarshipStatus::Destroyed:
    return StarShips::StarshipStatus::STARSHIP_STATUS_DESTROYED;
  }
  throw std::logic_error("invalid status: " + std::to_string(static_cast<int>(status)));
}

StarshipStatus from_protobuf(StarShips::StarshipStatus status)
{
  switch (status)
  {
  case StarShips::StarshipStatus::STARSHIP_STATUS_UNKNOWN:
    return StarshipStatus::Unknown;
  case StarShips::StarshipStatus::STARSHIP_STATUS_UNDER_CONSTRUCTION:
    return StarshipStatus::UnderConstruction;
  case StarShips::StarshipStatus::STARSHIP_STATUS_ON_STANDBY:
    return StarshipStatus::OnStandby;
  case StarShips::StarshipStatus::STARSHIP_STATUS_IN_ACTION:
    return StarshipStatus::InAction;
  case StarShips::StarshipStatus::STARSHIP_STATUS_DAMAGED:
    return StarshipStatus::Damaged;
  case StarShips::StarshipStatus::STARSHIP_STATUS_DESTROYED:
    return StarshipStatus::Destroyed;
  default:
    break;
  }
  throw std::logic_error("invalid status: " + std::to_string(static_cast<int>(status)));
}
void to_protobuf(const StarshipProperties& properties, StarShips::StarShip_Properties& protobuf_properties)
{
  protobuf_properties.set_id(properties.Id);
  protobuf_properties.set_name(properties.Name);
  protobuf_properties.set_description(properties.Description);
}

void to_protobuf(const Starship& starship, StarShips::StarShip& protobuf_starship)
{
  to_protobuf(starship.Properties, *protobuf_starship.mutable_properties());
  protobuf_starship.set_status(to_protobuf(starship.Status));
}

} // namespace

class GrpcHandler::Impl : public StarShips::StarShipService::Service
{
public:
  Impl(Api& api, const std::string& host, int port) : _api(api)
  {
    const std::string server_address(host + ":" + std::to_string(port));
    _serverBuilder.AddListeningPort(server_address, InsecureServerCredentials());
    _serverBuilder.RegisterService(this);
  }

  void Handle()
  {
    MSE_LOG_INFO("serving");
    _server = _serverBuilder.BuildAndStart();
    _server->Wait();
  }

  void Stop()
  {
    MSE_LOG_INFO("stop requested");
    _server->Shutdown();
  }

  virtual Status ListStarShips(::grpc::ServerContext* context, const ::StarShips::ListStarShipsRequest* /*request*/,
                               ::StarShips::ListStarShipsResponse* response)
  {
    return mse::ToGrpcStatus<grpc::Status>(
        mse::RequestHandler("listStarShips", mse::Context(mse::ToContextMetadata(context->client_metadata())))
            .With(mse::ClaimCheckerRequestHook::ScopeContains("read"))
            .With(mse::CachingRequestHook::Parameters(_cache)
                      .WithConstantResponse()
                      .WithCacheReader([response](const std::any& data) {
                        response->ParseFromString(std::any_cast<std::string>(data));
                      })
                      .WithCacheWriter([response]() -> std::any { return response->SerializeAsString(); })
                      .NeverExpire())
            .Process([&](mse::Context&) {
              for (const ::Starship& starship : _api.ListStarShips())
              {
                to_protobuf(starship, *response->add_starships());
              }
              return mse::Status::OK;
            }));
  }
  virtual Status GetStarShip(::grpc::ServerContext* context, const ::StarShips::GetStarShipRequest* request,
                             ::StarShips::GetStarShipResponse* response)
  {
    return mse::ToGrpcStatus<grpc::Status>(
        mse::RequestHandler("GetStarShip", mse::Context(mse::ToContextMetadata(context->client_metadata())))
            .With(mse::ClaimCheckerRequestHook::ScopeContains("read"))
            .Process([&](mse::Context&) {
              to_protobuf(_api.GetStarShip(request->id()), *response->mutable_starships());
              return mse::Status();
            }));
  }
  virtual Status UpdateStatus(::grpc::ServerContext* context, const ::StarShips::UpdateStatusRequest* request,
                              ::StarShips::UpdateStatusResponse* /*response*/)
  {
    return mse::ToGrpcStatus<grpc::Status>(
        mse::RequestHandler("UpdateStatus", mse::Context(mse::ToContextMetadata(context->client_metadata())))
            .With(mse::ClaimCheckerRequestHook::ScopeContains("write"))
            .Process([&](mse::Context&) {
              _api.UpdateStatus(request->id(), from_protobuf(request->status()));
              return mse::Status();
            }));
  }

private:
  Api& _api;
  std::unique_ptr<Server> _server;
  ServerBuilder _serverBuilder;
  std::shared_ptr<mse::Cache> _cache = std::make_shared<mse::UnorderedMapCache>();
};

GrpcHandler::GrpcHandler(Api& api, const std::string& host, int port) : _impl(std::make_unique<Impl>(api, host, port))
{
}

GrpcHandler::~GrpcHandler()
{
}

void GrpcHandler::Handle()
{
  _impl->Handle();
}

void GrpcHandler::Stop()
{
  _impl->Stop();
}
