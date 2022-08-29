#include "grpc-handler.h"
#include <generated/api.grpc.pb.h>
#include <grpcpp/grpcpp.h>

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

class ServiceImpl : public StarShips::StarShipService::Service
{
public:
    ServiceImpl(Api& api)
        : _api(api)
    {
    }

    virtual Status ListStarShips(::grpc::ServerContext* context, const ::StarShips::ListStarShipsRequest* request, ::StarShips::ListStarShipsResponse* response)
    {
        for(const ::Starship& starship : _api.ListStarShips())
        {            
            to_protobuf(starship, *response->add_starships());            
        }
        return Status::OK;
    }
    virtual Status GetStarShip(::grpc::ServerContext* context, const ::StarShips::GetStarShipRequest* request, ::StarShips::GetStarShipResponse* response)
    {
        to_protobuf(
            _api.GetStarShip(request->id()),
            *response->mutable_starships()
        );
        return Status::OK;
    }
    virtual Status UpdateStatus(::grpc::ServerContext* context, const ::StarShips::UpdateStatusRequest* request, ::StarShips::UpdateStatusResponse* response)
    {
        _api.UpdateStatus(request->id(), from_protobuf(request->status()) );
        return Status::OK;
    }
private:
    Api& _api;
};

}

GrpcHandler::GrpcHandler(Api& api, const std::string& host, int port)
    : _api(api)
    , _host(host)
    , _port(port)
{
}

GrpcHandler::~GrpcHandler()
{
}

void GrpcHandler::Handle()
{
    const std::string server_address(_host + ":" + std::to_string(_port));
    ServiceImpl service(_api);

    ServerBuilder builder;
    builder.AddListeningPort(server_address, InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());    
    
    server->Wait();    
}
