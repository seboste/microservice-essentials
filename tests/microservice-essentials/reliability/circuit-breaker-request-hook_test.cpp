#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <microservice-essentials/reliability/circuit-breaker-request-hook.h>

SCENARIO("MaxPendingRquestsExceededCircuitBreakerStrategy", "[reliability][circuit-breaker][request-hook]")
{
  struct TestCase
  {
    uint32_t pending_request_count;
    mse::CircuitBreakerStatus expected_status;
  };
  class TestStrategy : public mse::MaxPendingRquestsExceededCircuitBreakerStrategy
  {
  public:
    using MaxPendingRquestsExceededCircuitBreakerStrategy::MaxPendingRquestsExceededCircuitBreakerStrategy;

    void pre_process(mse::Context& ctx)
    {
      MaxPendingRquestsExceededCircuitBreakerStrategy::pre_process(ctx);
    }
    void post_process(mse::Context& ctx, mse::Status status)
    {
      MaxPendingRquestsExceededCircuitBreakerStrategy::post_process(ctx, status);
    }
  };

  GIVEN("a max pending request exceeded circuit breaker strategy with 2 allowed pending calls")
  {
    TestStrategy strategy(2);

    AND_GIVEN("an empty context")
    {
      mse::Context ctx;

      const std::vector<TestCase> test_cases = {{0, mse::CircuitBreakerStatus::CLOSED},
                                                {1, mse::CircuitBreakerStatus::CLOSED},
                                                {2, mse::CircuitBreakerStatus::CLOSED},
                                                {3, mse::CircuitBreakerStatus::OPEN}};
      for (const TestCase& tc : test_cases)
      {
        WHEN(std::to_string(tc.pending_request_count) + " requests are pending")
        {
          for (uint32_t i = 0; i < tc.pending_request_count; ++i)
          {
            strategy.pre_process(ctx);
          }
          THEN(
              (tc.expected_status == mse::CircuitBreakerStatus::CLOSED ? "the status is CLOSED" : "the status is OPEN"))
          {
            REQUIRE(strategy.GetStatus(ctx) == tc.expected_status);
          }
          for (uint32_t i = 0; i < tc.pending_request_count; ++i)
          {
            strategy.post_process(ctx, mse::Status::OK);
          }
          REQUIRE(strategy.GetStatus(ctx) == mse::CircuitBreakerStatus::CLOSED);
        }
      }
    }
    AND_GIVEN("a context with a request set")
    {
      mse::Context ctx({{"request", "A"}});
      mse::Context empty_ctx;
      mse::Context ctx_b({{"request", "B"}});

      const std::vector<TestCase> test_cases = {{0, mse::CircuitBreakerStatus::CLOSED},
                                                {1, mse::CircuitBreakerStatus::CLOSED},
                                                {2, mse::CircuitBreakerStatus::CLOSED},
                                                {3, mse::CircuitBreakerStatus::OPEN}};
      for (const TestCase& tc : test_cases)
      {
        WHEN(std::to_string(tc.pending_request_count) + " requests are pending")
        {
          for (uint32_t i = 0; i < tc.pending_request_count; ++i)
          {
            strategy.pre_process(ctx);
          }
          THEN(
              (tc.expected_status == mse::CircuitBreakerStatus::CLOSED ? "the status is CLOSED" : "the status is OPEN"))
          {
            REQUIRE(strategy.GetStatus(ctx) == tc.expected_status);
          }
          THEN("the other context status is CLOSED")
          {
            REQUIRE(strategy.GetStatus(ctx_b) == mse::CircuitBreakerStatus::CLOSED);
          }
          THEN("the empty context status is CLOSED")
          {
            REQUIRE(strategy.GetStatus(empty_ctx) == mse::CircuitBreakerStatus::CLOSED);
          }
          for (uint32_t i = 0; i < tc.pending_request_count; ++i)
          {
            strategy.post_process(ctx, mse::Status::OK);
          }
          REQUIRE(strategy.GetStatus(ctx) == mse::CircuitBreakerStatus::CLOSED);
        }
      }
    }
  }
}

SCENARIO("CircuitBreakerRequestHook", "[reliability][circuit-breaker][request-hook]")
{
  class DummyStrategy : public mse::CircuitBreakerStrategy
  {
  public:
    DummyStrategy(const mse::CircuitBreakerStatus& status) : _status(status)
    {
    }

    virtual mse::CircuitBreakerStatus GetStatus(const mse::Context&) const override
    {
      return _status;
    }

    virtual void pre_process(const mse::Context&) override
    {
      _preprocess_called++;
    }

    virtual void post_process(const mse::Context&, mse::Status) override
    {
      _postprocess_called++;
    }

    uint32_t _preprocess_called = 0;
    uint32_t _postprocess_called = 0;

  private:
    mse::CircuitBreakerStatus _status;
  };

  GIVEN("a circuit breaker with a strategy that is always closed")
  {
    std::shared_ptr<DummyStrategy> strategy = std::make_shared<DummyStrategy>(mse::CircuitBreakerStatus::CLOSED);
    mse::CircuitBreakerRequestHook hook({strategy, mse::Status{mse::StatusCode::failed_precondition, "test failure"}});

    WHEN("the process() method is called on something that returns OK")
    {
      bool is_processed = false;
      mse::Context ctx;
      mse::Status status = hook.Process(
          [&is_processed](mse::Context&) {
            is_processed = true;
            return mse::Status::OK;
          },
          ctx);

      THEN("the actual function is processed")
      {
        REQUIRE(is_processed == true);
      }
      THEN("pre and post process have been called once")
      {
        REQUIRE(strategy->_preprocess_called == 1);
        REQUIRE(strategy->_postprocess_called == 1);
      }
      THEN("status is OK")
      {
        REQUIRE(status.code == mse::StatusCode::ok);
      }
    }
    WHEN("the process() method is called on something that throws an exception")
    {
      bool is_processed = false;
      mse::Context ctx;
      REQUIRE_THROWS(hook.Process(
          [&is_processed](mse::Context&) {
            is_processed = true;
            throw std::runtime_error("some exception");
            return mse::Status::OK;
          },
          ctx));

      THEN("the actual function is processed")
      {
        REQUIRE(is_processed == true);
      }
      THEN("pre and post process have been called once")
      {
        REQUIRE(strategy->_preprocess_called == 1);
        REQUIRE(strategy->_postprocess_called == 1);
      }
    }
  }
  GIVEN("a circuit breaker with a strategy that is always open")
  {
    std::shared_ptr<DummyStrategy> strategy = std::make_shared<DummyStrategy>(mse::CircuitBreakerStatus::OPEN);
    mse::CircuitBreakerRequestHook hook({strategy, mse::Status{mse::StatusCode::failed_precondition, "test failure"}});
    {
      WHEN("the process() method is called on something that returns OK")
      {
        bool is_processed = false;
        mse::Context ctx;
        mse::Status status = hook.Process(
            [&is_processed](mse::Context&) {
              is_processed = true;
              return mse::Status::OK;
            },
            ctx);

        THEN("the actual function is NOT processed")
        {
          REQUIRE(is_processed == false);
        }
        THEN("pre and post process have been called once")
        {
          REQUIRE(strategy->_preprocess_called == 1);
          REQUIRE(strategy->_postprocess_called == 1);
        }
        THEN("configured status is called")
        {
          REQUIRE(status.code == mse::StatusCode::failed_precondition);
        }
      }
    }
  }
}