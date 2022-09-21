#pragma once

#include <string>
#include <map>

namespace mse
{

class Context
{
public:

    Context(Context* parent_context);
    Context();
    virtual ~Context();

    //static Context& GetGlobalContext();
    //static Context& GetThreadLocalContext();

    void Clear();

    typedef std::multimap<std::string, std::string> MetaData;    

    const MetaData& GetMetaData() const { return _metadata; }
    MetaData& GetMetaData() { return _metadata; }
    MetaData GetAllMetaData() const;
    
    void Insert(const std::string& key, const std::string& value);        
    const std::string& At(const std::string& key) const;
    bool Contains(const std::string& key) const;

private:
    Context* _parent_context;

    MetaData _metadata;
};

}
