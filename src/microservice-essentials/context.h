#pragma once

#include <string>
#include <map>
#include <initializer_list>


namespace mse
{

class Context
{
public:
    typedef std::multimap<std::string, std::string> MetaData;    

    Context();
    Context(std::initializer_list<MetaData::value_type> metadata);
    Context(Context* parent_context);
    Context(Context* parent_context, std::initializer_list<MetaData::value_type> metadata);
    
    virtual ~Context();

    //static Context& GetGlobalContext();
    //static Context& GetThreadLocalContext();

    void Clear();

    const MetaData& GetMetaData() const { return _metadata; }
    MetaData& GetMetaData() { return _metadata; }
    MetaData GetAllMetaData() const;
    
    void Insert(std::initializer_list<MetaData::value_type> metadata);
    void Insert(const std::string& key, const std::string& value);        
    const std::string& At(const std::string& key) const;
    bool Contains(const std::string& key) const;

private:
    Context* _parent_context = nullptr;
    MetaData _metadata;
};

}
