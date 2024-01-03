#include <catch2/catch_test_macros.hpp>
#include <microservice-essentials/performance/caching-request-hook.h>
#include <thread>

using namespace std::chrono_literals;

namespace
{
class DummyCache : public mse::Cache
{
public:
  DummyCache() = default;
  virtual ~DummyCache() = default;

  virtual void Insert(const mse::Cache::Hash& hash, const Element& e) override
  {
    hashToBeInserted = hash;
    elementToBeInserted = e;
  }

  virtual void Remove(const mse::Cache::Hash& hash) override
  {
    hashToBeRemoved = hash;
  }

  virtual mse::Cache::Element Get(const mse::Cache::Hash&) const override
  {
    return element;
  }

  mse::Cache::Hash hashToBeInserted;
  mse::Cache::Element elementToBeInserted;

  mse::Cache::Hash hashToBeRemoved;
  mse::Cache::Element element;
};
} // namespace

SCENARIO("Caching Request Hook", "[performance][caching][request-hook]")
{
  GIVEN("A caching request hook")
  {
    int object = 25;
    std::shared_ptr<DummyCache> cache = std::make_shared<DummyCache>();
    mse::CachingRequestHook hook(mse::CachingRequestHook::Parameters(cache)
                                     .Include(mse::StatusCode::cancelled)
                                     .WithHasher([]() { return 1; })
                                     .WithCachedObject(object)
                                     .WithMaxAge(std::chrono::milliseconds(5)));

    AND_GIVEN("an empty cache")
    {
      WHEN("process is called on a successful function")
      {
        bool hasBeenCalled = false;
        auto before = mse::Cache::Clock::now();
        mse::Context context;
        hook.Process(
            [&object, &hasBeenCalled](const mse::Context&) {
              object = 42;
              hasBeenCalled = true;
              return mse::Status::OK;
            },
            context);

        THEN("the function is called")
        {
          REQUIRE(hasBeenCalled == true);
        }

        THEN("the cache contains the element")
        {
          REQUIRE(cache->hashToBeInserted == 1);
          REQUIRE(cache->elementToBeInserted.status == mse::Status::OK);
          REQUIRE(cache->elementToBeInserted.insertion_time <= mse::Cache::Clock::now());
          REQUIRE(cache->elementToBeInserted.insertion_time >= before);
          REQUIRE(std::any_cast<int>(cache->elementToBeInserted.data) == 42);
        }
      }
      WHEN("process is called on a failing function that returns not_found")
      {
        mse::Context context;
        hook.Process([](const mse::Context&) { return mse::Status{mse::StatusCode::not_found, "not found"}; }, context);

        THEN("the cache does not contain the element")
        {
          REQUIRE(cache->hashToBeInserted == mse::Cache::Hash{});
          REQUIRE(mse::Cache::IsValid(cache->elementToBeInserted) == false);
        }
      }
      WHEN("process is called on a failing function that returns cancelled")
      {
        mse::Context context;
        hook.Process([](const mse::Context&) { return mse::Status{mse::StatusCode::cancelled, "cancelled"}; }, context);

        THEN("the cache contains the element")
        {
          REQUIRE(cache->hashToBeInserted == 1);
          REQUIRE(cache->elementToBeInserted.status.code == mse::StatusCode::cancelled);
        }
      }
    }

    AND_GIVEN("a cache with one element")
    {
      cache->element = mse::Cache::Element{91, mse::Status::OK, mse::Cache::Clock::now()};

      WHEN("process is called right away")
      {
        bool hasBeenCalled = false;
        mse::Context context;
        hook.Process(
            [&object, &hasBeenCalled](const mse::Context&) {
              object = 42;
              hasBeenCalled = true;
              return mse::Status::OK;
            },
            context);

        THEN("the function is not called")
        {
          REQUIRE(hasBeenCalled == false);
        }

        THEN("the object is restored from the cache")
        {
          REQUIRE(object == 91);
        }
      }

      WHEN("process is called after the max age")
      {
        bool hasBeenCalled = false;
        auto before = mse::Cache::Clock::now();
        mse::Context context;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        hook.Process(
            [&object, &hasBeenCalled](const mse::Context&) {
              object = 42;
              hasBeenCalled = true;
              return mse::Status::OK;
            },
            context);

        THEN("the function is called")
        {
          REQUIRE(hasBeenCalled == true);
        }

        THEN("the object is restored from the cache")
        {
          REQUIRE(object == 42);
        }

        THEN("the new value has been inserted into the cache")
        {
          REQUIRE(cache->hashToBeInserted == 1);
          REQUIRE(cache->elementToBeInserted.status == mse::Status::OK);
          REQUIRE(cache->elementToBeInserted.insertion_time <= mse::Cache::Clock::now());
          REQUIRE(cache->elementToBeInserted.insertion_time >= before);
          REQUIRE(std::any_cast<int>(cache->elementToBeInserted.data) == 42);
        }

        THEN("the old value has been removed from the cache")
        {
          REQUIRE(cache->hashToBeRemoved == 1);
        }
      }
    }
  }
}

SCENARIO("Unorded Map Cache", "[performance][caching]")
{
  GIVEN("a unordered map cache")
  {
    mse::UnorderedMapCache cache;
    WHEN("a an element is inserted")
    {
      cache.Insert(1, mse::Cache::Element{1, mse::Status::OK, mse::Cache::Clock::now()});
      THEN("the element can be retrieved")
      {
        auto element = cache.Get(1);
        REQUIRE(mse::Cache::IsValid(element) == true);
        REQUIRE(element.status == mse::Status::OK);
        REQUIRE(std::any_cast<int>(element.data) == 1);
      }

      AND_WHEN("an element is removed")
      {
        cache.Remove(1);
        THEN("the element cannot be retrieved")
        {
          auto element = cache.Get(1);
          REQUIRE(mse::Cache::IsValid(element) == false);
        }
      }
    }
    WHEN("a non existing element is removed")
    {
      cache.Remove(1);
      THEN("the element cannot be retrieved")
      {
        auto element = cache.Get(1);
        REQUIRE(mse::Cache::IsValid(element) == false);
      }
    }
  }
}

SCENARIO("LRU Cache", "[performance][caching]")
{
  GIVEN("a LRU cache with an unordered map cache as the backend")
  {
    std::shared_ptr<mse::UnorderedMapCache> realCache = std::make_shared<mse::UnorderedMapCache>();
    mse::LRUCache cache(realCache, 2);
    WHEN("an element is inserted")
    {
      cache.Insert(1, mse::Cache::Element{1, mse::Status::OK, mse::Cache::Clock::now()});
      THEN("the element can be retrieved")
      {
        auto element = cache.Get(1);
        REQUIRE(mse::Cache::IsValid(element) == true);
        REQUIRE(element.status == mse::Status::OK);
        REQUIRE(std::any_cast<int>(element.data) == 1);
      }

      AND_WHEN("an element is removed")
      {
        cache.Remove(1);
        THEN("the element cannot be retrieved")
        {
          auto element = cache.Get(1);
          REQUIRE(mse::Cache::IsValid(element) == false);
        }
      }
    }
    WHEN("a non existing element is removed")
    {
      cache.Remove(1);
      THEN("the element cannot be retrieved")
      {
        auto element = cache.Get(1);
        REQUIRE(mse::Cache::IsValid(element) == false);
      }
    }
    WHEN("the cache is full")
    {
      cache.Insert(1, mse::Cache::Element{1, mse::Status::OK, mse::Cache::Clock::now()});
      cache.Insert(2, mse::Cache::Element{2, mse::Status::OK, mse::Cache::Clock::now()});
      cache.Get(1); // 1 is now the most recently used element, 2 the least recently used
      cache.Insert(3, mse::Cache::Element{3, mse::Status::OK, mse::Cache::Clock::now()});
      THEN("the least recently used element is removed")
      {
        REQUIRE(mse::Cache::IsValid(cache.Get(1)) == true);
        REQUIRE(mse::Cache::IsValid(cache.Get(2)) == false);
        REQUIRE(mse::Cache::IsValid(cache.Get(3)) == true);
      }
    }
  }
}
