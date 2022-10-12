#include "grpc-handler.h"
#include <generated/api.grpc.pb.h>
#include <grpcpp/grpcpp.h>
#include <microservice-essentials/handler.h>
#include <microservice-essentials/context.h>
#include <microservice-essentials/observability/logger.h>
#include <microservice-essentials/request/request-processor.h>
#include <microservice-essentials/utilities/metadata-converter.h>

using namespace grpc;

namespace {

StarShips::StarshipStatus to_protobuf(StarshipStatus status)
{
   switch(status)
    {
        case StarshipStatus::Unknown: return StarShips::StarshipStatus::STARSHIP_STATUS_UNKNOWN;
        case StarshipStatus::UnderConstruction: return StarShips::StarshipStatus::STARSHIP_STATUS_UNDER_CONSTRUCTION;
        case StarshipStatus::OnStandby: return StarShips::StarshipStatus::STARSHIP_STATUS_ON_STANDBY;
        case StarshipStatus::InAction: return StarShips::StarshipStatus::STARSHIP_STATUS_IN_ACTION;
        case StarshipStatus::Damaged: return StarShips::StarshipStatus::STARSHIP_STATUS_DAMAGED;
        case StarshipStatus::Destroyed: return StarShips::StarshipStatus::STARSHIP_STATUS_DESTROYED;
    }
    throw std::logic_error("invalid status: " + std::to_string(static_cast<int>(status)));
}

StarshipStatus from_protobuf(StarShips::StarshipStatus status)
{
    switch(status)
    {
        case StarShips::StarshipStatus::STARSHIP_STATUS_UNKNOWN: return StarshipStatus::Unknown;
        case StarShips::StarshipStatus::STARSHIP_STATUS_UNDER_CONSTRUCTION: return StarshipStatus::UnderConstruction;
        case StarShips::StarshipStatus::STARSHIP_STATUS_ON_STANDBY: return StarshipStatus::OnStandby;
        case StarShips::StarshipStatus::STARSHIP_STATUS_IN_ACTION: return StarshipStatus::InAction;
        case StarShips::StarshipStatus::STARSHIP_STATUS_DAMAGED: return StarshipStatus::Damaged;
        case StarShips::StarshipStatus::STARSHIP_STATUS_DESTROYED: return StarshipStatus::Destroyed;
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

} //end anon ns

class GrpcHandler::Impl : public StarShips::StarShipService::Service, mse::Handler
{
public:
    Impl(Api& api, const std::string& host, int port)
        : _api(api)
    {
        const std::string server_address(host + ":" + std::to_string(port));
        _serverBuilder.AddListeningPort(server_address, InsecureServerCredentials());
        _serverBuilder.RegisterService(this);
    }

    virtual void Handle() override
    {
        MSE_LOG_INFO("serving");
        _server = _serverBuilder.BuildAndStart();
        _server->Wait();
    }

    virtual void Stop() override
    {
        MSE_LOG_INFO("stop requested");
        _server->Shutdown();
    }

    virtual Status ListStarShips(::grpc::ServerContext* context, const ::StarShips::ListStarShipsRequest* request, ::StarShips::ListStarShipsResponse* response)
    {
        mse::RequestHandler("listStarShips", mse::Context(mse::ToContextMetadata(context->client_metadata())))
            .Process([&](mse::Context& context)
            {
                MSE_LOG_TRACE("handling list starships request");
                for(const ::Starship& starship : _api.ListStarShips())
                {            
                    to_protobuf(starship, *response->add_starships());            
                }
                return mse::Status();
            });
        return Status::OK;
    }
    virtual Status GetStarShip(::grpc::ServerContext* context, const ::StarShips::GetStarShipRequest* request, ::StarShips::GetStarShipResponse* response)
    {
        mse::RequestHandler("GetStarShip", mse::Context(mse::ToContextMetadata(context->client_metadata())))
            .Process([&](mse::Context& context)
            {
                MSE_LOG_TRACE("handling get starship request");

                to_protobuf(
                    _api.GetStarShip(request->id()),
                    *response->mutable_starships()
                );
                return mse::Status();
            });
        return Status::OK;
    }
    virtual Status UpdateStatus(::grpc::ServerContext* context, const ::StarShips::UpdateStatusRequest* request, ::StarShips::UpdateStatusResponse* response)
    {
        mse::RequestHandler("UpdateStatus", mse::Context(mse::ToContextMetadata(context->client_metadata())))
            .Process([&](mse::Context& context)
            {
                MSE_LOG_TRACE("handling update status request");
                
                _api.UpdateStatus(request->id(), from_protobuf(request->status()) );
                return mse::Status();
            });
        return Status::OK;
    }
private:
    Api& _api;
    std::unique_ptr<Server> _server;
    ServerBuilder _serverBuilder;
};

GrpcHandler::GrpcHandler(Api& api, const std::string& host, int port)
    : _impl(std::make_unique<Impl>(api, host, port))
{
}

GrpcHandler::~GrpcHandler()
{
}

void GrpcHandler::Handle()
{
    _impl->Handle();
}
