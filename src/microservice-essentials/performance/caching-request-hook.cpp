#include "caching-request-hook.h"
#include <mutex>

using namespace mse;

const Cache::Element Cache::InvalidElement{std::any(), Status{StatusCode::unknown, "invalid cached element"},
                                           Cache::TimePoint::min()};
bool Cache::IsValid(const Element& element)
{
  return element.data.has_value();
}

CachingRequestHook::Parameters::Parameters(std::shared_ptr<Cache> cache_) : cache(cache_)
{
}

CachingRequestHook::Parameters& CachingRequestHook::Parameters::WithConstantResponse()
{
  key_generator = []() -> std::string { return std::string{}; };
  return *this;
}

CachingRequestHook::Parameters& CachingRequestHook::Parameters::WithKeyGenerator(const CacheKeyGen& key_generator_)
{
  key_generator = key_generator_;
  return *this;
}

CachingRequestHook::Parameters& CachingRequestHook::Parameters::WithKey(const std::string& key_)
{
  key_generator = [key_]() -> std::string { return key_; };
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

CachingRequestHook::Parameters& CachingRequestHook::Parameters::IncludeAllStatusCodes()
{
  for (int status_code_as_int = static_cast<int>(mse::StatusCode::lowest);
       status_code_as_int <= static_cast<int>(mse::StatusCode::highest); ++status_code_as_int)
  {
    status_codes_to_cache.insert(static_cast<mse::StatusCode>(status_code_as_int));
  }
  return *this;
}

CachingRequestHook::Parameters& CachingRequestHook::Parameters::Include(const StatusCode& status_code_)
{
  status_codes_to_cache.insert(status_code_);
  return *this;
}

CachingRequestHook::Parameters& CachingRequestHook::Parameters::Include(
    const std::initializer_list<StatusCode>& status_codes_)
{
  for (const StatusCode& status_code : status_codes_)
  {
    status_codes_to_cache.insert(status_code);
  }
  return *this;
}

CachingRequestHook::Parameters& CachingRequestHook::Parameters::Exclude(const StatusCode& status_code_)
{
  status_codes_to_cache.erase(status_code_);
  return *this;
}

CachingRequestHook::Parameters& CachingRequestHook::Parameters::Exclude(
    const std::initializer_list<StatusCode>& status_codes_)
{
  for (const StatusCode& status_code : status_codes_)
  {
    status_codes_to_cache.erase(status_code);
  }
  return *this;
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
  std::string key = _parameters.key_generator();

  if (Cache::Element element = _parameters.cache->Get(key); Cache::IsValid(element))
  {
    if ((Cache::Clock::now() - element.insertion_time) <= _parameters.max_age)
    {
      // cache hit
      _parameters.cache_reader(element.data);
      return element.status;
    }
    else
    {
      // cache expired
      _parameters.cache->Remove(key);
    }
  }

  // cache miss
  Status status = func(context);
  if (_parameters.status_codes_to_cache.find(status.code) != _parameters.status_codes_to_cache.end())
  {
    _parameters.cache->Insert(key, Cache::Element{_parameters.cache_writer(), status, Cache::Clock::now()});
  }
  return status;
}

void UnorderedMapCache::Insert(const std::string& key, const Element& element)
{
  std::unique_lock lock(_mutex);
  _data[key] = element;
}

Cache::Element UnorderedMapCache::Get(const std::string& key) const
{
  std::shared_lock lock(_mutex);
  if (const auto& cit = _data.find(key); cit != _data.end())
  {
    return cit->second;
  }
  return InvalidElement;
}

void UnorderedMapCache::Remove(const std::string& key)
{
  std::unique_lock lock(_mutex);
  _data.erase(key);
}

LRUCache::LRUCache(std::shared_ptr<Cache> realCache, std::size_t maxSize) : _realCache(realCache), _maxSize(maxSize)
{
}

void LRUCache::Insert(const std::string& key, const Element& element)
{
  if (_realCache == nullptr)
  {
    return;
  }

  std::unique_lock lock(_mutex);

  // 1. ensure that there is enought space in the cache
  while (_lru.size() >= _maxSize)
  {
    _realCache->Remove(_lru.back());
    _lru.pop_back();
  }

  // 2. insert element into the cache
  _lru.push_front(key);
  _realCache->Insert(key, Element{LRUElement{_lru.begin(), element.data}, element.status, element.insertion_time});
}

Cache::Element LRUCache::Get(const std::string& key) const
{
  if (_realCache == nullptr)
  {
    return Cache::InvalidElement;
  }

  // 1. move element to the very front of the list
  std::unique_lock lock(_mutex);
  Cache::Element element = _realCache->Get(key);
  if (!Cache::IsValid(element))
  {
    return Cache::InvalidElement;
  }
  LRUElement lruElement = std::any_cast<LRUElement>(element.data);
  _lru.splice(_lru.begin(), _lru, lruElement.first);

  // 2. return element
  return Element{lruElement.second, element.status, element.insertion_time};
}

void LRUCache::Remove(const std::string& key)
{
  if (_realCache == nullptr)
  {
    return;
  }
  std::unique_lock lock(_mutex);
  mse::Cache::Element element = _realCache->Get(key);
  if (!mse::Cache::IsValid(element))
  {
    return;
  }
  if (auto it = std::any_cast<LRUElement>(element.data).first; it != _lru.end())
  {
    _lru.erase(it);
  }
  _realCache->Remove(key);
}
