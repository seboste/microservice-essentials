#pragma once

#include <microservice-essentials/context.h>

namespace mse
{
    
    /**
     * Converts metadata from a container to Context::Metadata
     * can be used for all containers that hold a pair of some string class that can be
     * converted to a std::string using std::string(std::begin(my_string), std::end(my_string))
     * works for e.g. grpc::ServerContext::client_metadata() and httplib::Headers
     * 
     * Optionally, all keys can be converted to lower case. That facilitates the handling of case 
     * insensitivity, which is required for http. In grpc, metadata keys should be lower case anyway
     */
    template<typename Container> inline Context::Metadata ToContextMetadata(const Container& external_metadata, bool keys_to_lower = true);

    /**
     * Converts Context::Metadata to some container
     * can be used for all container types that have an import-method for a key-value-pair of strings (e.g. std::set, std::multimap, std::map, ...)
     * works for e.g. httplib::Headers
     */
    template<typename Container> inline Container FromContextMetadata(const Context::Metadata& metadata);

    /**
     * Exports metadata by calling some function that takes two strings for each metadata item
     * works for e.g. grpc::ClientContext::AddMetaData 
     * 
     * Code snippet:
     * mse::Context ctx;
     * grpc::ClientContext cc;
     * mse::ExportMetadata(ctx.GetMetadata(), &grpc::ClientContext::AddMetadata, cc);     
     */
    template<typename ExportFunction> inline void ExportMetadata(const Context::Metadata& metadata, ExportFunction export_fn);
    template<typename ExportMemberFunction, typename ExportObjectType> inline void ExportMetadata(const Context::Metadata& metadata, ExportMemberFunction export_fn, ExportObjectType& export_obj);    
}

#include "metadata-converter.txx"
