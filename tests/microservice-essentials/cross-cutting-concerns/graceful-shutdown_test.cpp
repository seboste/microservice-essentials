#include <catch2/catch_test_macros.hpp>
#include <microservice-essentials/cross-cutting-concerns/graceful-shutdown.h>
#include <thread>

using namespace std::chrono_literals;

SCENARIO("Graceful Shutdown Registration & Callback Invocation", "[cross-cutting-concerns][graceful-shutdown]")
{
  GIVEN("Graceful Shutdown with one callback registered")
  {
    std::atomic<int> cbCallCount = 0;
    mse::GracefulShutdown::GetInstance().Register("test_cb", [&]() { cbCallCount++; });

    WHEN("Shutdown is invoked")
    {
      mse::GracefulShutdown::GetInstance().RequestShutdown();
      THEN("the callback is called")
      {
        REQUIRE(cbCallCount == 1);
      }
    }

    WHEN("a second callback is registered and shutdown is invoked")
    {
      bool hasBeenCalled2 = false;
      mse::GracefulShutdown::GetInstance().Register("test2_cb", [&]() { hasBeenCalled2 = true; });
      mse::GracefulShutdown::GetInstance().RequestShutdown();
      THEN("both callbacks are called")
      {
        REQUIRE(cbCallCount == 1);
        REQUIRE(hasBeenCalled2 == true);
      }
      mse::GracefulShutdown::GetInstance().UnRegister("test2_cb");
    }

    WHEN("the callback is unregistered and shutdown is invoked")
    {
      mse::GracefulShutdown::GetInstance().UnRegister("test_cb");
      mse::GracefulShutdown::GetInstance().RequestShutdown();
      THEN("the callback is not called anymore")
      {
        REQUIRE(cbCallCount == 0);
      }
    }

    WHEN("the a callback is registered with the same id")
    {
      THEN("a runtime error occurs")
      {
        REQUIRE_THROWS_AS(mse::GracefulShutdown::GetInstance().Register("test_cb", []() {}), std::runtime_error);
      }
    }
    mse::GracefulShutdown::GetInstance().UnRegister("test_cb");
  }
}

/* disabled flaky test on MacOS
SCENARIO("Graceful Shutdown Registration & Callback Invocation",
         "[cross-cutting-concerns][graceful-shutdown][signal-handler]")
{
  GIVEN("a GracefulShutdownOnSignal instance connected to the SHUTDOWN signal")
  {
    std::atomic<int> cbCallCount = 0;
    mse::GracefulShutdown::GetInstance().Register("test_cb", [&]() { cbCallCount++; });

    mse::GracefulShutdownOnSignal gracefulShutdown;

    WHEN("the shutdown signal is raised")
    {
      std::raise(static_cast<int>(mse::Signal::SIG_SHUTDOWN));
      THEN("the callback is called within 30ms")
      {
        std::this_thread::sleep_for(30ms);
        REQUIRE(cbCallCount == 1);
      }
    }

    mse::GracefulShutdown::GetInstance().UnRegister("test_cb");
  }
}
*/
