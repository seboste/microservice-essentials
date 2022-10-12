#pragma once

#include "status-converter.h"
#include <exception>
#include <type_traits>


namespace mse
{

template<typename GrpcStatusCode> 
inline GrpcStatusCode ToGrpcStatusCode(const StatusCode& status_code)
{
    if(status_code == StatusCode::invalid)
    {
        throw std::invalid_argument("cannot convert StatusCode::invalid");
    }

    return static_cast<GrpcStatusCode>(static_cast<int>(status_code));
}

template<typename GrpcStatusCode> 
inline StatusCode FromGrpcStatusCode(const GrpcStatusCode& grpc_status_code)
{
    int code = static_cast<int>(grpc_status_code);
    if(code < static_cast<int>(StatusCode::lowest) || code > static_cast<int>(StatusCode::highest))
    {
        throw std::out_of_range(std::string("cannot convert code ") + std::to_string(code) + "to StatusCode");
    }
    return static_cast<StatusCode>(code);
}

template<typename GrpcStatus> 
inline GrpcStatus ToGrpcStatus(const Status& status)
{        
    return GrpcStatus(ToGrpcStatusCode<decltype(GrpcStatus().error_code())>(status.code), status.details);
}

template<typename GrpcStatus> 
inline Status FromGrpcStatus(const GrpcStatus& grpc_status)
{
    return Status { FromGrpcStatusCode(grpc_status.error_code()), grpc_status.error_message() };
}

}
