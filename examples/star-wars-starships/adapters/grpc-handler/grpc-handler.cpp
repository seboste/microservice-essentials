#include "grpc-handler.h"
#include <generated/api.grpc.pb.h>
#include <grpcpp/grpcpp.h>

using namespace grpc;

namespace {
class ServiceImpl : public StarShips::StarShipService::Service
{
public:
    ServiceImpl(Api& api)
        : _api(api)
    {
    }

    virtual Status ListStarShips(::grpc::ServerContext* context, const ::StarShips::ListStarShipsRequest* request, ::StarShips::ListStarShipsResponse* response)
    {
        std::cout << "ListStarShips" << std::endl;
        return Status::OK;
    }
    virtual Status GetStarShip(::grpc::ServerContext* context, const ::StarShips::GetStarShipRequest* request, ::StarShips::GetStarShipResponse* response)
    {
        std::cout << "GetStarShip" << std::endl;
        return Status::OK;
    }
    virtual Status UpdateStatus(::grpc::ServerContext* context, const ::StarShips::UpdateStatusRequest* request, ::StarShips::UpdateStatusResponse* response)
    {
        std::cout << "UpdateStatus" << std::endl;
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
    std::string server_address("0.0.0.0:50051");
    ServiceImpl service(_api);

    ServerBuilder builder;
    builder.AddListeningPort(server_address, InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}

#if 0
void listStarShips(const httplib::Request& request, httplib::Response& response);
    void getStarShip(const httplib::Request& request, httplib::Response& response);
    void updateStatus(const httplib::Request& request, httplib::Response& response);
namespace {

using json = nlohmann::json;

std::string to_string(StarshipStatus status)
{
    switch(status)
    {
        case StarshipStatus::Unknown: return "Unknown";
        case StarshipStatus::UnderConstruction: return "UnderConstruction";
        case StarshipStatus::OnStandby: return "OnStandby";
        case StarshipStatus::InAction: return "InAction";
        case StarshipStatus::Damaged: return "Damaged";
        case StarshipStatus::Destroyed: return "Destroyed";
    }
    throw std::logic_error("invalid status: " + std::to_string(static_cast<int>(status)));
}

StarshipStatus from_string(const std::string& statusString)
{
    static const std::unordered_map<std::string, StarshipStatus> lookupMap =
    {
        { "Unknown" , StarshipStatus::Unknown },
        { "UnderConstruction" , StarshipStatus::UnderConstruction },
        { "OnStandby" , StarshipStatus::OnStandby },
        { "InAction" , StarshipStatus::InAction },
        { "Damaged" , StarshipStatus::Damaged },
        { "Destroyed" , StarshipStatus::Destroyed },
    };

    const auto cit = lookupMap.find(statusString);
    if(cit == lookupMap.end())
    {
        throw std::logic_error("invalid status: " + statusString);
    }
    return cit->second;    
}

json to_json(const Starship& starship)
{
    return json { 
        { "properties", 
            {
                { "id", starship.Properties.Id },
                { "name", starship.Properties.Name },
                { "description", starship.Properties.Description }
            }
        },
        {
            "status", to_string(starship.Status)
        }
    };
}

json to_json(const std::vector<Starship>& starships)
{
    json jsonStarShips = json::array();
    for(const Starship& starship : starships)
    {
        jsonStarShips.emplace_back(to_json(starship));
    }
    return jsonStarShips;
}

std::string extractId(const std::string& path)
{
    std::smatch ip_result;
    std::regex_match(path, ip_result, std::regex("/(.*)/(.*)"));
    if(ip_result.size() != 3)
    {        
        throw std::invalid_argument("invalid URL");
    }
    
    return ip_result[2];    
}

}

HttpHandler::HttpHandler(Api& api, const std::string& host, int port)
    : _api(api)
    , _svr(std::make_unique<httplib::Server>())
    , _host(host)
    , _port(port)
{    
    _svr->Get("/StarShips", std::bind(&HttpHandler::listStarShips, this, std::placeholders::_1, std::placeholders::_2));
    _svr->Get("/StarShip/(.*)", std::bind(&HttpHandler::getStarShip, this, std::placeholders::_1, std::placeholders::_2));
    _svr->Put("/StarShipStatus/(.*)", httplib::Server::Handler(std::bind(&HttpHandler::updateStatus, this, std::placeholders::_1, std::placeholders::_2)));
}

HttpHandler::~HttpHandler()
{
}

void HttpHandler::Handle()
{
    _svr->listen(_host, _port);
}

void HttpHandler::listStarShips(const httplib::Request& request, httplib::Response& response)
{   
    response.set_content(
            to_json(_api.ListStarShips()).dump(),
            "text/json"
        );
    response.status = 200;
}

void HttpHandler::getStarShip(const httplib::Request& request, httplib::Response& response)
{
    response.set_content(
            to_json(_api.GetStarShip(extractId(request.path))).dump(),
            "text/json"
        );
    response.status = 200;
}

void HttpHandler::updateStatus(const httplib::Request& request, httplib::Response& response)
{
    _api.UpdateStatus(
        extractId(request.path),
        from_string(json::parse(request.body).at("status"))
        );
    response.status = 200;
}

#endif