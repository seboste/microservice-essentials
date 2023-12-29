#pragma once

#include <any>
#include <chrono>
#include <memory>
#include <microservice-essentials/request/request-hook-factory.h>
#include <microservice-essentials/request/request-hook.h>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>

namespace mse
{

class Cache
{
public:
  using Clock = std::chrono::steady_clock;
  using TimePoint = std::chrono::time_point<Clock>;
  using Hash = std::size_t;
  struct Element
  {
    std::any data;
    Status status;
    TimePoint insertion_time;
  };
  static const Element InvalidElement;
  static bool IsValid(const Element& element);

  virtual void Insert(const Hash& hash, const Status& status, const std::any& element) = 0;
  virtual Element Get(const Hash& hash) const = 0;
  virtual void Remove(const Hash& hash) = 0;
};

using CacheHasher = std::function<Cache::Hash()>;         // generates a hash for the request
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
    Parameters& WithHasher(const CacheHasher& hasher_);
    template <typename T> Parameters& WithStdHasher(const T& object); // uses std::hash<T> to generate the hash

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
    CacheHasher hasher;
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

class UnorderedMapCache : public Cache
{
public:
  virtual void Insert(const Hash& hash, const Status& status, const std::any& element) override;
  virtual Element Get(const Hash& hash) const override;
  virtual void Remove(const Hash& hash) override;

private:
  mutable std::shared_mutex _mutex;
  std::unordered_map<Hash, Element> _data;
};

template <typename T> CachingRequestHook::Parameters& CachingRequestHook::Parameters::WithCachedObject(T& object)
{
  cache_reader = [&object](const std::any& data) { object = std::any_cast<T>(data); };
  cache_writer = [&object]() -> std::any { return T(object); };
  return *this;
}

template <typename T> CachingRequestHook::Parameters& CachingRequestHook::Parameters::WithStdHasher(const T& object)
{
  hasher = [&object]() -> Cache::Hash { return std::hash<T>{}(object); };
  return *this;
}

} // namespace mse
