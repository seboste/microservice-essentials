#pragma once

#include <chrono>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <initializer_list>


#define MSE_LOCAL_CONTEXT mse::Context(nullptr, __FILE__, __FUNCTION__, __LINE__)

namespace mse
{

/**
 * Class that typically holds request specific metadata (string->string) to be used throughout the request.
 * 
 * A global context instance is accessible via the static method GetGlobalContext(). It holds metadata that is valid 
 * independent from the request (e.g. the name of the application)
 * 
 * A thread local context instance is accessible via the static method GetThreadLocalContext(). It holds metadata that is valid 
 * for the whole thread. As each request is typically handled in a dedicated thread, this instance can be used from anywhere
 * without explicitly passing the context around. See the usage in the example's handler and client classes.
 * 
 * Each context has a parent that can be specified during construction. If no parent is specified (i.e. nullptr) the thread local
 * context is used as the parent. The thread local context's parent is the global context by default.
 * Use GetAllMetaData() to get the combined metadata of the the instance, its parent, its parent's parent, and so on.
 * The Insert()- and Contains()- method operates on GetAllMetaData()
 * 
 * Utilities/metadata-converter can be used to convert from and to technology specific context equivalent objects.
 */
class Context
{
public:
    typedef std::multimap<std::string, std::string> Metadata;
    typedef std::vector<Metadata::value_type> MetadataVector;

    Context(const Metadata& metadata, const Context* parent_context);
    Context(const Context* parent_context);
    Context(const Metadata& metadata);
    Context(std::initializer_list<Metadata::value_type> metadata, const Context* parent_context);
    Context(std::initializer_list<Metadata::value_type> metadata);
    Context(const Context* parent_context, const std::string& file, const std::string& function, int line, std::chrono::time_point<std::chrono::system_clock> tp = std::chrono::system_clock::now());
    Context();
    Context(const Context& other_context);
    Context(Context&& other_context);
    virtual ~Context();

    Context& operator=(const Context& other_context);
    Context& operator=(Context&& other_context);

    static Context& GetGlobalContext();
    static Context& GetThreadLocalContext();

    void Clear();

    const Metadata& GetMetadata() const { return _metadata; }
    Metadata& GetMetadata() { return _metadata; }
    Metadata GetAllMetadata() const;
    MetadataVector GetFilteredMetadata(const std::vector<std::string>& keys) const;
    
    void Insert(std::initializer_list<Metadata::value_type> metadata);
    void Insert(const std::string& key, const std::string& value);        
    const std::string& At(const std::string& key) const;
    const std::string& AtOr(const std::string& key, const std::string& default_value) const;
    bool Contains(const std::string& key) const;

private:
    class NoParent {};
    Context(const NoParent& no_parent);

    void initParentContext(const Context& other_context);
    std::set<const Context*> getAllParents() const;

    Metadata _metadata;
    const Context* _parent_context = nullptr;
};

}
