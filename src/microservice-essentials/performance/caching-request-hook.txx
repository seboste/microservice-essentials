#include <microservice-essentials/performance/caching-request-hook.h>

namespace mse
{

template <typename T> CachingRequestHook::Parameters& CachingRequestHook::Parameters::WithCachedObject(T& object)
{
  cache_reader = [&object](const std::any& data) { object = std::any_cast<T>(data); };
  cache_writer = [&object]() -> std::any { return T(object); };
  return *this;
}

template <typename T>
CachingRequestHook::Parameters& CachingRequestHook::Parameters::WithStdToStringKeyGenerator(const T& object)
{
  key_generator = [&object]() -> std::string { return std::to_string(object); };
  return *this;
}

} // namespace mse