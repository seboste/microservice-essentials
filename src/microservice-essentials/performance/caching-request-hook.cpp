#include "caching-request-hook.h"
#include <microservice-essentials/observability/logger.h>
#include <mutex>

using namespace mse;

const Cache::Element Cache::InvalidElement{std::any(), Cache::TimePoint::min()};
bool Cache::IsValid(const Element& element)
{
  return element.data.has_value();
}

CachingRequestHook::Parameters::Parameters(std::shared_ptr<Cache> cache_) : cache(cache_)
{
}

CachingRequestHook::Parameters& CachingRequestHook::Parameters::WithConstantResponse()
{
  hasher = []() -> mse::Cache::Hash { return 0; };
  return *this;
}

CachingRequestHook::Parameters& CachingRequestHook::Parameters::WithHasher(const CacheHasher& hasher_)
{
  hasher = hasher_;
  return *this;
}

CachingRequestHook::Parameters& CachingRequestHook::Parameters::WithCacheReader(CacheReader reader_)
{
  cache_reader = reader_;
  return *this;
}

CachingRequestHook::Parameters& CachingRequestHook::Parameters::WithCacheWriter(CacheWriter writer_)
{
  cache_writer = writer_;
  return *this;
}

CachingRequestHook::Parameters& CachingRequestHook::Parameters::WithMaxAge(const Duration& max_age_)
{
  max_age = max_age_;
  return *this;
}

CachingRequestHook::Parameters& CachingRequestHook::Parameters::NeverExpire()
{
  return WithMaxAge(Duration::max());
}

CachingRequestHook::CachingRequestHook(const Parameters& parameters)
    : mse::RequestHook("caching"), _parameters(parameters)
{
}

CachingRequestHook::~CachingRequestHook()
{
}

Status CachingRequestHook::Process(Func func, Context& context)
{
  Cache::Hash hash = _parameters.hasher();

  if (Cache::Element element = _parameters.cache->Get(hash);
      Cache::IsValid(element) && (Cache::Clock::now() - element.insertion_time) <= _parameters.max_age)
  {
    // cache hit
    MSE_LOG_DEBUG("cache hit");
    _parameters.cache_reader(element.data);
    return Status::OK;
  }
  else
  {
    // cache miss
    MSE_LOG_DEBUG("cache miss");
    Status status = func(context);
    if (status)
    {
      _parameters.cache->Insert(hash, _parameters.cache_writer());
    }
    return status;
  }
}

void UnorderedMapCache::Insert(const Hash& hash, const std::any& element)
{
  std::unique_lock lock(_mutex);
  _data[hash] = Element{element, Clock::now()};
}

Cache::Element UnorderedMapCache::Get(const Hash& hash) const
{
  std::shared_lock lock(_mutex);
  if (const auto& cit = _data.find(hash); cit != _data.end())
  {
    return cit->second;
  }
  return InvalidElement;
}

void UnorderedMapCache::Remove(const Hash& hash)
{
  std::unique_lock lock(_mutex);
  _data.erase(hash);
}
