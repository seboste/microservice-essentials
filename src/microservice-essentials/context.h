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

class Context
{
public:
    typedef std::multimap<std::string, std::string> Metadata;
    typedef std::vector<Metadata::value_type> MetadataVector;

    Context(const Metadata& metadata, const Context* parent_context);
    Context(const Context* parent_context) : Context({}, parent_context) {}
    Context(const Metadata& metadata) : Context(metadata, nullptr) {}
    Context(std::initializer_list<Metadata::value_type> metadata, const Context* parent_context) : Context(Metadata(metadata), parent_context) {}
    Context(std::initializer_list<Metadata::value_type> metadata) : Context(metadata, nullptr) {}
    Context(const Context* parent_context, const std::string& file, const std::string& function, int line, std::chrono::time_point<std::chrono::system_clock> tp = std::chrono::system_clock::now());
    Context() : Context({}, nullptr) {}
    Context(const Context& other_context) { *this = other_context; }
    Context(Context&& other_context) { *this = std::move(other_context); }
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
    bool Contains(const std::string& key) const;

private:
    class NoParent {};
    Context(const NoParent& no_parent);

    void initParentContext(const Context& other_context);
    std::set<const Context*> getAllParents() const;

    const Context* _parent_context = nullptr;
    Metadata _metadata;
};

}
