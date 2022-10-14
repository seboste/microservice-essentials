#pragma once

#include <microservice-essentials/status.h>
#include <unordered_map>

namespace mse
{
    
    /**
     * Converts a status code from and to grpc status code
     * Done by a simple cast as the StatusCode values are identical to grpc StatusCode values by design
     * Throws an std::invalid argument exception when trying to convert StatusCode::invalid 
     * Throws an std::out_of_range if the grpc status code is not within the value range of StatusCode
     */
    template<typename GrpcStatusCode> inline GrpcStatusCode ToGrpcStatusCode(const StatusCode& status_code);
    template<typename GrpcStatusCode> inline StatusCode FromGrpcStatusCode(const GrpcStatusCode& grpc_status_code);

    template<typename GrpcStatus> inline GrpcStatus ToGrpcStatus(const Status& status);
    template<typename GrpcStatus> inline Status FromGrpcStatus(const GrpcStatus& grpc_status);


    /**
     * Converts a status code from and to HTTP status code
     * Done by using look up tables in both directions. Default look up tables are available
     * Throws an std::invalid argument exception when trying to convert StatusCode::invalid 
     * Throws an std::out_of_range if the HTTP status code is not present in the look up table
     */
    typedef std::unordered_map<mse::StatusCode, int> StatusCodeToHttpStatusCodeLookupTable;
    
     //first map describes a generic mapping (e.g. 2xx -> mse::StatusCode::ok). Key is the first digit (e.g. 2 for 2xx)
     //second map describes mappings for individual codes (e.g. 501 -> mse::StatusCode::unimplemented)
    typedef std::pair<std::unordered_map<int, mse::StatusCode>,std::unordered_map<int, mse::StatusCode>> HttpStatusCodeToStatusCodeLookupTables;
    

    extern HttpStatusCodeToStatusCodeLookupTables DefaultHttpStatusCodeToStatusCodeLookupTables;
    extern StatusCodeToHttpStatusCodeLookupTable DefaultStatusCodeToHttpStatusCodeLookupTable;

    StatusCode FromHttpStatusCode(int http_status_code, const HttpStatusCodeToStatusCodeLookupTables& lut = DefaultHttpStatusCodeToStatusCodeLookupTables);
    int ToHttpStatusCode(const StatusCode& status_code, const StatusCodeToHttpStatusCodeLookupTable& lut = DefaultStatusCodeToHttpStatusCodeLookupTable);
    
}


#include "status-converter.txx"