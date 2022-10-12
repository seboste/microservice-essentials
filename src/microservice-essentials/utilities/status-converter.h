#pragma once

#include <microservice-essentials/status.h>

namespace mse
{
    
    /**
     * Converts a status code from and to grpc status code
     * Done by a simple cast as the StatusCode values are identical to grpc StatusCode values by design
     * Throws an std::invalid argument exception when trying to convert StatusCode::invalid 
     * Throws an std::out_of_range if the grpc status code is not with the value range of StatusCode
     */
    template<typename GrpcStatusCode> inline GrpcStatusCode ToGrpcStatusCode(const StatusCode& status_code);
    template<typename GrpcStatusCode> inline StatusCode FromGrpcStatusCode(const GrpcStatusCode& grpc_status_code);

    template<typename GrpcStatus> inline GrpcStatus ToGrpcStatus(const Status& status);
    template<typename GrpcStatus> inline Status FromGrpcStatus(const GrpcStatus& grpc_status);

}


#include "status-converter.txx"