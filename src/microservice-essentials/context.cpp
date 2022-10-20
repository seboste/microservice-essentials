#include "context.h"

#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <time.h>

using namespace mse;

namespace 
{

std::string to_string(std::chrono::time_point<std::chrono::system_clock> tp)
{
    std::time_t tt = std::chrono::system_clock::to_time_t(tp);
    std::tm tm;
#ifdef _MSC_VER //doesn't have gmtime_r and gmtime is already thread safe
    gmtime_s(&tm, &tt); //non standard - reverse order of arguments
#else    
    gmtime_r(&tt, &tm); //GMT (UTC)
#endif
    std::stringstream ss;
    ss << std::put_time( &tm, "%FT%TZ" );
    return ss.str();
}

}

Context::Context(const NoParent&)
    : _parent_context(nullptr)
{
}

Context::Context(const Context* parent_context)
    : Context({}, parent_context)
{
}

Context::Context(const Metadata& metadata)
    : Context(metadata, nullptr)
{
}

Context::Context(std::initializer_list<Metadata::value_type> metadata, const Context* parent_context) 
    : Context(Metadata(metadata), parent_context)
{
}

Context::Context(std::initializer_list<Metadata::value_type> metadata)
    : Context(metadata, nullptr)
{
}

Context::Context(const Metadata& metadata, const Context* parent_context)
    : _metadata(metadata)
    , _parent_context(parent_context
        ? parent_context
        : &GetThreadLocalContext() //let the thread local context be the parent
    )
{
    if(parent_context == this) //I cannot be my own parent
    {
        throw std::logic_error("parent context is identical to its child");
    }
}

Context::Context(const Context* parent_context, const std::string& file, const std::string& function, int line, std::chrono::time_point<std::chrono::system_clock> tp)
    : Context({ { "file", file }, {"function", function}, { "line", std::to_string(line) }, { "timestamp", to_string(tp)} }, parent_context)
{
}


Context::Context() 
    : Context({}, nullptr) 
{
}

Context::Context(const Context& other_context)
{ 
    *this = other_context; 
}

Context::Context(Context&& other_context) 
{ 
    *this = std::move(other_context);
}

Context::~Context()
{
}

Context& Context::operator=(const Context& other_context)
{
    if(this == &other_context)
    {
        return *this; //nothing todo
    }

    _metadata = other_context._metadata;
    initParentContext(other_context);
    return *this;
}

Context& Context::operator=(Context&& other_context)
{
    if(this == &other_context)
    {
        return *this; //nothing todo
    }

    _metadata = std::move(other_context._metadata);
    initParentContext(other_context);
    return *this;
}

Context& Context::GetGlobalContext()
{
    static Context global_context{NoParent()}; //global context must not have a parent. All others do.
    return global_context;
}

Context& Context::GetThreadLocalContext()
{
    static thread_local Context threadlocal_context(&GetGlobalContext()); //global context is the parent of the thread local context
    return threadlocal_context;
}

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

const std::string& Context::AtOr(const std::string& key, const std::string& default_value) const
{
    if(auto cit = _metadata.find(key); cit != _metadata.cend())
    {
        return cit->second;
    }
    if(_parent_context != nullptr)
    {
        return _parent_context->AtOr(key, default_value);
    }
    return default_value;
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

std::set<const Context*> Context::getAllParents() const
{    
    std::set<const Context*> parents;
    if(_parent_context != nullptr)
    {
        parents.insert(_parent_context);
        parents.merge(_parent_context->getAllParents());
    }
    return parents;
}

void Context::initParentContext(const Context& other_context)
{    
    if(&GetGlobalContext() == this) //global context must never have a parent
    {
        _parent_context = nullptr;
    }    
    else
    {
        if(other_context._parent_context != this) //don't change _parent_context if other_context is a child
        {
            if(other_context.getAllParents().count(this))
            {
                throw std::logic_error("I cannot be one of my ancestors");
            }
            _parent_context = other_context._parent_context;
        }
    }
}
