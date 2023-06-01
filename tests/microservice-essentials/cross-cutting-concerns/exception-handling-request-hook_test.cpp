#include <catch2/catch_test_macros.hpp>
#include <microservice-essentials/cross-cutting-concerns/exception-handling-request-hook.h>
#include <utilities/history_logger.h>

SCENARIO("Exception Handling Request Hook Creation", "[cross-cutting-concerns][exception handling][request-hook]")
{
  WHEN("a exception handling request hook is created based on the parameters")
  {
    std::unique_ptr<mse::RequestHook> exception_handling_request_hook =
        mse::RequestHookFactory::GetInstance().Create(mse::ExceptionHandlingRequestHook::Parameters());
    THEN("the request hook is not null")
    {
      REQUIRE(exception_handling_request_hook != nullptr);
    }
  }
}

SCENARIO("Exception Handling Definition", "[cross-cutting-concerns][exception handling]")
{
  GIVEN("Exception Type Mapper for specific type")
  {
    mse::ExceptionHandling::ExceptionOfTypeMapper<std::runtime_error> runtime_error_mapper(
        mse::ExceptionHandling::Definition{});
    WHEN("the specific type is tested")
    {
      std::optional<mse::ExceptionHandling::Definition> definition =
          runtime_error_mapper.Map(std::make_exception_ptr(std::runtime_error("runtime error")));
      THEN("test result is not empty")
      {
        REQUIRE(definition.has_value());
      }
    }
    WHEN("a derived type is tested")
    {
      std::optional<mse::ExceptionHandling::Definition> definition =
          runtime_error_mapper.Map(std::make_exception_ptr(std::range_error("range error")));
      THEN("test result is not empty")
      {
        REQUIRE(definition.has_value());
      }
    }
    WHEN("base exception type is tested")
    {
      std::optional<mse::ExceptionHandling::Definition> definition =
          runtime_error_mapper.Map(std::make_exception_ptr(std::exception()));
      THEN("test result is empty")
      {
        REQUIRE(!definition.has_value());
      }
    }
    WHEN("a completely unrelated type is tested")
    {
      std::optional<mse::ExceptionHandling::Definition> definition =
          runtime_error_mapper.Map(std::make_exception_ptr(10));
      THEN("test result is empty")
      {
        REQUIRE(!definition.has_value());
      }
    }
  }

  GIVEN("Constant Mapper")
  {
    mse::ExceptionHandling::ToConstantMapper to_constant_mapper(mse::ExceptionHandling::Definition{});
    WHEN("some random exception is tested")
    {
      std::optional<mse::ExceptionHandling::Definition> definition =
          to_constant_mapper.Map(std::make_exception_ptr(10));
      THEN("test result is not empty")
      {
        REQUIRE(definition.has_value());
      }
    }
  }
}

SCENARIO("Exception Handling Request Hook", "[cross-cutting-concerns][exception handling][request-hook]")
{

  GIVEN("Exception Handling Request Hook with single exception handling definition")
  {
    using namespace mse::ExceptionHandling;
    mse::ExceptionHandlingRequestHook request_hook(
        mse::ExceptionHandlingRequestHook::Parameters({std::make_shared<ExceptionOfTypeMapper<std::runtime_error>>(
            Definition{mse::Status{mse::StatusCode::data_loss, "test"}, mse::LogLevel::invalid, false})}));

    WHEN("the registered exception is thrown")
    {
      mse::Context context;
      mse::Status status = request_hook.Process(
          [](mse::Context&) {
            throw std::runtime_error("some runtime error");
            return mse::Status::OK;
          },
          context);
      THEN("registered status is returned")
      {
        REQUIRE(status.code == mse::StatusCode::data_loss);
        REQUIRE(status.details == "test");
      }
    }
    WHEN("some other exception is thrown")
    {
      mse::Context context;
      THEN("exception is passed along")
      {
        REQUIRE_THROWS_AS(request_hook.Process(
                              [](mse::Context&) {
                                throw std::logic_error("some logic error");
                                return mse::Status::OK;
                              },
                              context),
                          std::logic_error);
      };
    }
    WHEN("some error is returned")
    {
      mse::Context context;
      mse::Status status = request_hook.Process(
          [](mse::Context&) {
            return mse::Status{mse::StatusCode::not_found, "test"};
          },
          context);
      THEN("the error is propagated")
      {
        REQUIRE(status.code == mse::StatusCode::not_found);
        REQUIRE(status.details == "test");
      }
    }
  }

  GIVEN("Exception Handling Request Hook with default exception to status matching")
  {
    mse::ExceptionHandlingRequestHook request_hook(mse::ExceptionHandlingRequestHook::Parameters{});
    WHEN("invalid argument exception is thrown")
    {
      mse::Context context;
      mse::Status status = request_hook.Process(
          [](mse::Context&) {
            throw std::invalid_argument("some exception details");
            return mse::Status::OK;
          },
          context);
      THEN("invalid_argument status is returned")
      {
        REQUIRE(status.code == mse::StatusCode::invalid_argument);
        REQUIRE(status.details == "invalid argument exception: some exception details");
      }
    }
    WHEN("out of range excption is thrown")
    {
      mse::Context context;
      mse::Status status = request_hook.Process(
          [](mse::Context&) {
            throw std::out_of_range("some exception details");
            return mse::Status::OK;
          },
          context);
      THEN("invalid_argument status is returned")
      {
        REQUIRE(status.code == mse::StatusCode::out_of_range);
        REQUIRE(status.details == "out of range argument exception: some exception details");
      }
    }
    WHEN("some other exception is thrown")
    {
      mse::Context context;
      mse::Status status =
          request_hook.Process([](mse::Context&) -> mse::Status { throw std::string("bla"); }, context);
      THEN("internal status is returned")
      {
        REQUIRE(status.code == mse::StatusCode::internal);
        REQUIRE(status.details == "unknown exception");
      }
    }
  }

  GIVEN("a history logger and an Exception Handling Request Hook with and without logging exception handling")
  {
    using namespace mse::ExceptionHandling;
    mse::Context context;
    mse_test::HistoryLogger log;
    mse::ExceptionHandlingRequestHook request_hook(mse::ExceptionHandlingRequestHook::Parameters({
        std::make_shared<ExceptionOfTypeMapper<std::logic_error>>(
            Definition{mse::Status::OK, mse::LogLevel::invalid, false}), // no logging
        std::make_shared<ExceptionOfTypeMapper<std::runtime_error>>(
            Definition{mse::Status::OK, mse::LogLevel::trace, false}) // logging
    }));

    WHEN("non logging exception is thrown")
    {
      request_hook.Process(
          [](mse::Context&) {
            throw std::logic_error("my custom exception details");
            return mse::Status::OK;
          },
          context);

      THEN("log does not contain my custom exception details")
      {
        REQUIRE(log._log_history[mse::LogLevel::trace].empty());
      }
    }
    WHEN("logging exception is thrown")
    {
      request_hook.Process(
          [](mse::Context&) {
            throw std::runtime_error("my custom exception details");
            return mse::Status::OK;
          },
          context);

      THEN("log contains my custom exception details")
      {
        REQUIRE(log._log_history[mse::LogLevel::trace].size() == 1);
        REQUIRE(log._log_history[mse::LogLevel::trace][0].find("my custom exception details") != std::string::npos);
      }
    }
  }

  GIVEN("Exception Handling Request Hook with and without detail forwarding")
  {
    using namespace mse::ExceptionHandling;
    mse::Context context;
    mse::ExceptionHandlingRequestHook request_hook(mse::ExceptionHandlingRequestHook::Parameters({
        std::make_shared<ExceptionOfTypeMapper<std::logic_error>>(
            Definition{mse::Status::OK, mse::LogLevel::invalid, true}), // detail forwarding
        std::make_shared<ExceptionOfTypeMapper<std::runtime_error>>(
            Definition{mse::Status::OK, mse::LogLevel::invalid, false}) // no detail forwarding
    }));

    WHEN("non forwarding exception is thrown")
    {
      mse::Status status = request_hook.Process(
          [](mse::Context&) {
            throw std::runtime_error("my custom exception details");
            return mse::Status::OK;
          },
          context);

      THEN("details do not contain my custom exception details")
      {
        std::cout << "details: " << status.details << std::endl;
        REQUIRE(status.details.find("my custom exception details") == std::string::npos);
      }
    }
    WHEN("forwarding exception is thrown")
    {
      mse::Status status = request_hook.Process(
          [](mse::Context&) {
            throw std::logic_error("my custom exception details");
            return mse::Status::OK;
          },
          context);

      THEN("details contain my custom exception details")
      {
        REQUIRE(status.details.find("my custom exception details") != std::string::npos);
      }
    }
  }
}