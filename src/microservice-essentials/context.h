#pragma once

#include <string>
#include <map>
#include <initializer_list>


namespace mse
{

class Context
{
public:
    typedef std::multimap<std::string, std::string> Metadata;    
    
    Context(const Metadata& metadata, Context* parent_context);
    Context(Context* parent_context) : Context({}, parent_context) {}
    Context(const Metadata& metadata) : Context(metadata, nullptr) {}
    Context(std::initializer_list<Metadata::value_type> metadata, Context* parent_context) : Context(Metadata(metadata), parent_context) {}
    Context(std::initializer_list<Metadata::value_type> metadata) : Context(metadata, nullptr) {}
    Context() : Context({}, nullptr) {}

    virtual ~Context();

    //static Context& GetGlobalContext();
    //static Context& GetThreadLocalContext();

    void Clear();

    const Metadata& GetMetaData() const { return _metadata; }
    Metadata& GetMetaData() { return _metadata; }
    Metadata GetAllMetaData() const;
    
    void Insert(std::initializer_list<Metadata::value_type> metadata);
    void Insert(const std::string& key, const std::string& value);        
    const std::string& At(const std::string& key) const;
    bool Contains(const std::string& key) const;

private:
    Context* _parent_context = nullptr;
    Metadata _metadata;
};

}

#include "context.txx"