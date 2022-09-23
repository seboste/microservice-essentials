#include "context.h"

#include <sstream>
#include <iomanip>

using namespace mse;

namespace 
{

std::string to_string(std::chrono::time_point<std::chrono::system_clock> tp)
{
    std::time_t tt = std::chrono::system_clock::to_time_t(tp);
    std::tm tm = *std::gmtime(&tt); //GMT (UTC)
    std::stringstream ss;
    ss << std::put_time( &tm, "%FT%TZ" );
    return ss.str();
}


}

Context::Context(const Metadata& metadata, const Context* parent_context)
    : _metadata(metadata)
    , _parent_context(parent_context)
{
}

Context::Context(const Context* parent_context, const std::string& file, const std::string& function, int line, std::chrono::time_point<std::chrono::system_clock> tp)
    : Context({ { "file", file }, {"function", function}, { "line", std::to_string(line) }, { "timestamp", to_string(tp)} }, parent_context)
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

Context::Metadata Context::GetAllMetadata() const
{
    Context::Metadata metadata = _metadata;
    if(_parent_context != nullptr)
    {
        metadata.merge(_parent_context->GetAllMetadata());
    }
    return metadata;
}

Context::MetadataVector Context::GetFilteredMetadata(const std::vector<std::string>& keys) const
{
    const Metadata all_metadata = GetAllMetadata();
    MetadataVector filtered_metadata;
    for(const auto& key : keys)
    {
        if(auto cit = all_metadata.find(key); cit != all_metadata.end())
        {
            filtered_metadata.push_back(*cit);
        }        
    }
    return filtered_metadata;
}
    
void Context::Insert(const std::string& key, const std::string& value)
{
    _metadata.insert({ key, value});
}

void Context::Insert(std::initializer_list<Metadata::value_type> metadata)
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
