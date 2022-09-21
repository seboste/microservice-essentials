#include "context.h"

using namespace mse;

Context::Context(const MetaData& metadata, Context* parent_context)
    : _metadata(metadata)
    , _parent_context(parent_context)
{
}

Context::~Context()
{
}


    //static Context& GetGlobalContext();
    //static Context& GetThreadLocalContext();

void Context::Clear()
{
    _metadata.clear();
}

Context::MetaData Context::GetAllMetaData() const
{
    Context::MetaData metadata = _metadata;
    if(_parent_context != nullptr)
    {
        metadata.merge(_parent_context->GetAllMetaData());
    }
    return metadata;
}
    
void Context::Insert(const std::string& key, const std::string& value)
{
    _metadata.insert({ key, value});
}

void Context::Insert(std::initializer_list<MetaData::value_type> metadata)
{
    _metadata.insert(metadata);
}

const std::string& Context::At(const std::string& key) const
{
    if(auto cit = _metadata.find(key); cit != _metadata.cend())
    {
        return cit->second;
    }
    if(_parent_context != nullptr)
    {
        return _parent_context->At(key);
    }
    throw std::out_of_range(key + " not found in context metadata");
}

bool Context::Contains(const std::string& key) const
{
    if(_metadata.find(key) != _metadata.cend())
    {
        return true;
    }
    if(_parent_context != nullptr)
    {
        return _parent_context->Contains(key);
    }
    return false;
}
