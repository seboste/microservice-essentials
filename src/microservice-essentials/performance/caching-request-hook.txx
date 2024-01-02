#include <microservice-essentials/performance/caching-request-hook.h>

namespace mse
{

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

}