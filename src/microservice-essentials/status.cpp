#include "status.h"
#include <stdexcept>
#include <unordered_map>

using namespace mse;

mse::Status mse::Status::OK = mse::Status();

std::string mse::to_string(StatusCode status_code)
{
    switch(status_code)  
    {
        case StatusCode::ok : return "OK";
        case StatusCode::cancelled : return "CANCELLED";
        case StatusCode::unknown : return "UNKNOWN";
        case StatusCode::invalid_argument : return "INVALID_ARGUMENT";
        case StatusCode::deadline_exceeded : return "DEADLINE_EXCEEDED";
        case StatusCode::not_found : return "NOT_FOUND";
        case StatusCode::already_exists : return "ALREADY_EXISTS";
        case StatusCode::permission_deinied : return "PERMISSION_DENIED";
        case StatusCode::resource_exhausted : return "RESOURCE_EXHAUSTED";
        case StatusCode::failed_precondition : return "FAILED_PRECONDITION";
        case StatusCode::aborted : return "ABORTED";
        case StatusCode::out_of_range : return "OUT_OF_RANGE";
        case StatusCode::unimplemented : return "UNIMPLEMENTED";
        case StatusCode::internal : return "INTERNAL";
        case StatusCode::unavailable : return "UNAVAILABLE";
        case StatusCode::data_loss : return "DATA_LOSS";
        case StatusCode::unauthenticated : return "UNAUTHENTICATED";
    }
    throw std::invalid_argument(std::string("invalid status code with id ") + std::to_string(static_cast<int>(status_code)));    
}

void mse::from_string(const std::string& status_code_string, StatusCode& status_code)
{
 static const std::unordered_map<std::string, StatusCode> mapping =
    {
        { "OK" , StatusCode::ok },
        { "CANCELLED" , StatusCode::cancelled },
        { "UNKNOWN" , StatusCode::unknown },
        { "INVALID_ARGUMENT" , StatusCode::invalid_argument },
        { "DEADLINE_EXCEEDED" , StatusCode::deadline_exceeded },
        { "NOT_FOUND" , StatusCode::not_found },
        { "ALREADY_EXISTS" , StatusCode::already_exists },
        { "PERMISSION_DENIED" , StatusCode::permission_deinied },
        { "RESOURCE_EXHAUSTED" , StatusCode::resource_exhausted },
        { "FAILED_PRECONDITION" , StatusCode::failed_precondition },
        { "ABORTED" , StatusCode::aborted },
        { "OUT_OF_RANGE" , StatusCode::out_of_range },
        { "UNIMPLEMENTED" , StatusCode::unimplemented },
        { "INTERNAL" , StatusCode::internal },
        { "UNAVAILABLE" , StatusCode::unavailable },
        { "DATA_LOSS" , StatusCode::data_loss },
        { "UNAUTHENTICATED" , StatusCode::unauthenticated }
    };
    
    auto cit = mapping.find(status_code_string); 
    status_code = (cit != mapping.cend())
        ? cit->second
        : StatusCode::invalid;
}

bool operator>>(std::istream& is, mse::StatusCode& status_code)
{
    status_code = mse::StatusCode::invalid;
    std::string status_code_string;
    is >> status_code_string;
    mse::from_string(status_code_string, status_code);
    return status_code != mse::StatusCode::invalid;
}

std::ostream& operator<<(std::ostream& os, const mse::StatusCode& status_code)
{
    os << mse::to_string(status_code);
    return os;
}
