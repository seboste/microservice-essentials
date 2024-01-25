#pragma once

#include <any>
#include <chrono>
#include <list>
#include <memory>
#include <microservice-essentials/request/request-hook-factory.h>
#include <microservice-essentials/request/request-hook.h>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>

namespace mse
{

/**
 * Cache interface with insert, get and remove operations.
 */
class Cache
{
public:
  using Clock = std::chrono::steady_clock;
  using TimePoint = std::chrono::time_point<Clock>;
  struct Element
  {
    std::any data;
    Status status;
    TimePoint insertion_time;
  };
  static const Element InvalidElement;
  static bool IsValid(const Element& element);

  virtual ~Cache() = default;

  virtual void Insert(const std::string& key, const Element& element) = 0;
  virtual Element Get(const std::string& key) const = 0;
  virtual void Remove(const std::string& key) = 0;
};

using CacheKeyGen = std::function<std::string()>;         // generates a key for the request
using CacheReader = std::function<void(const std::any&)>; // restores the object from the cache
using CacheWriter = std::function<std::any()>;            // returns the data to be cached

/**
 * Request hook that returns immediately if the requested resource is already in the cache.
 */
class CachingRequestHook : public mse::RequestHook
{
public:
  struct Parameters
  {
    using Duration = std::chrono::duration<double, std::milli>;

    Parameters(std::shared_ptr<Cache> cache_);

    Parameters& WithConstantResponse();
    Parameters& WithKeyGenerator(const CacheKeyGen& key_generator_);
    Parameters& WithKey(const std::string& key_);
    template <typename T>
    Parameters& WithStdToStringKeyGenerator(const T& object); // uses std::to_string<T> to generate the key

    Parameters& WithCacheReader(CacheReader reader_);
    Parameters& WithCacheWriter(CacheWriter writer_);
    template <typename T> Parameters& WithCachedObject(T& o); // sets writer and reader to store and restore the object

    Parameters& WithMaxAge(const Duration& max_age_);
    Parameters& NeverExpire(); // same as WithMaxAge(std::chrono::duration<double>::max())

    Parameters& IncludeAllStatusCodes();
    Parameters& Include(const StatusCode& status_code_);
    Parameters& Include(const std::initializer_list<StatusCode>& status_codes_);
    Parameters& Exclude(const StatusCode& status_code_);
    Parameters& Exclude(const std::initializer_list<StatusCode>& status_codes_);

    std::shared_ptr<Cache> cache;
    CacheKeyGen key_generator;
    CacheReader cache_reader;
    CacheWriter cache_writer;
    Duration max_age = std::chrono::minutes(10);
    std::unordered_set<StatusCode> status_codes_to_cache = {StatusCode::ok};

    AutoRequestHookParameterRegistration<CachingRequestHook::Parameters, CachingRequestHook> auto_registration;
  };

  CachingRequestHook(const Parameters& parameters);
  virtual ~CachingRequestHook();

  virtual Status Process(Func func, Context& context) override;

protected:
private:
  Parameters _parameters;
};

/**
 * Cache implementation based on a std::unordered_map.
 */
class UnorderedMapCache : public Cache
{
public:
  UnorderedMapCache() = default;
  virtual ~UnorderedMapCache() = default;

  virtual void Insert(const std::string& key, const Element& element) override;
  virtual Element Get(const std::string& key) const override;
  virtual void Remove(const std::string& key) override;

private:
  mutable std::shared_mutex _mutex;
  std::unordered_map<std::string, Element> _data;
};

/**
 * Cache decorator that implements a Least Recently Used (LRU) cache.
 */
class LRUCache : public Cache
{
public:
  LRUCache(std::shared_ptr<Cache> realCache, std::size_t maxSize = 1000);
  virtual ~LRUCache() = default;

  virtual void Insert(const std::string& key, const Element& element) override;
  virtual Element Get(const std::string& key) const override;
  virtual void Remove(const std::string& key) override;

private:
  typedef std::pair<std::list<std::string>::iterator, std::any> LRUElement;
  std::shared_ptr<Cache> _realCache;
  mutable std::shared_mutex _mutex;
  mutable std::list<std::string> _lru;
  std::size_t _maxSize = 1000;
};

} // namespace mse

#include <microservice-essentials/performance/caching-request-hook.txx>
