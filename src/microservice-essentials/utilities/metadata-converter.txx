#pragma once

#include "metadata-converter.h"

namespace mse
{

template<typename Container> 
inline Context::Metadata ToContextMetadata(const Container& external_metadata)
{
    Context::Metadata metadata;
    for(auto key_value_pair: external_metadata)
    {
        const std::string key(std::begin(key_value_pair.first), std::end(key_value_pair.first));
        const std::string value(std::begin(key_value_pair.second), std::end(key_value_pair.second));
        metadata.insert({ key, value });
    }
    return metadata;
}

}

