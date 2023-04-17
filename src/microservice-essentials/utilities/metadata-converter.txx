#pragma once

#include "metadata-converter.h"
#include <functional>
#include <algorithm>
#include <cctype>
#include <string>

namespace mse
{

inline std::string tolower(std::string str)
{    
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c){ return std::tolower(c); });
    return str;
}

template<typename Container> 
inline Context::Metadata ToContextMetadata(const Container& external_metadata, bool keys_to_lower)
{
    Context::Metadata metadata;
    for(const auto& key_value_pair: external_metadata)
    {
        std::string key(std::begin(key_value_pair.first), std::end(key_value_pair.first));
        if(keys_to_lower)
        {
            key = tolower(key);
        }
        const std::string value(std::begin(key_value_pair.second), std::end(key_value_pair.second));
        metadata.insert({ key, value });
    }
    return metadata;
}

template<typename Container> 
inline Container FromContextMetadata(const Context::Metadata& metadata, const std::set<std::string>& fields)
{
    Container external_metadata;
    for(const auto& key_value_pair : metadata)
    {
        if(fields.find(tolower(key_value_pair.first)) != fields.end())
        {
            external_metadata.insert({key_value_pair.first, key_value_pair.second});
        }
    }
    return external_metadata;
}

template<typename ExportFunction>
inline void ExportMetadata(const Context::Metadata& metadata, ExportFunction export_fn, const std::set<std::string>& fields)
{
    for(const auto& key_value_pair : metadata)
    {
        if(fields.find(tolower(std::string(key_value_pair.first))) != fields.end())
        {
            export_fn(key_value_pair.first, key_value_pair.second);
        }
    }
}

template<typename ExportMemberFunction, typename ExportObjectType>
inline void ExportMetadata(const Context::Metadata& metadata, ExportMemberFunction export_fn, ExportObjectType& export_obj, const std::set<std::string>& fields)
{
    using namespace std::placeholders;
    ExportMetadata(metadata, std::bind(export_fn, &export_obj, _1, _2), fields);        
}

}

