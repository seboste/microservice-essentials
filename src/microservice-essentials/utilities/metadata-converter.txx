#pragma once

#include "metadata-converter.h"

namespace mse
{

template<typename Container> 
inline Context::Metadata ToContextMetadata(const Container& external_metadata)
{
    Context::Metadata metadata;
    for(const auto& key_value_pair: external_metadata)
    {
        const std::string key(std::begin(key_value_pair.first), std::end(key_value_pair.first));
        const std::string value(std::begin(key_value_pair.second), std::end(key_value_pair.second));
        metadata.insert({ key, value });
    }
    return metadata;
}

template<typename Container> 
inline Container FromContextMetadata(const Context::Metadata& metadata)
{
    Container external_metadata;
    for(const auto& key_value_pair : metadata)
    {
        external_metadata.insert({key_value_pair.first, key_value_pair.second});
    }
    return external_metadata;
}

}

