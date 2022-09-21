#pragma once

#include <microservice-essentials/context.h>

namespace mse
{
    
    /**
     * Converts metadata from a container to Context::MetaData
     * can be used for all containers that hold a pair of some string class that can be
     * converted to a std::string using std::string(std::begin(my_string), std::end(my_string))
     * works for e.g. grpc::ServerContext::client_metadata()
     */
    template<typename Container> inline Context::MetaData ToContextMetadata(const Container& external_metadata);
}

#include "metadata-converter.txx"
