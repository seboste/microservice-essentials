#include <catch2/catch_test_macros.hpp>
#include <microservice-essentials/request/request-hook-factory.h>

namespace
{

class TestRequestHook : public mse::RequestHook
{
public:
  struct Parameters
  {
    int a = 0;
    int b = 1;
  };

  TestRequestHook(const Parameters& /*parameters*/) : mse::RequestHook("test")
  {
  }

  static std::unique_ptr<mse::RequestHook> Create(const std::any& parameters)
  {
    return std::make_unique<TestRequestHook>(std::any_cast<Parameters>(parameters));
  }

private:
};

} // namespace

SCENARIO("RequestHookFactory", "[request]")
{
  GIVEN("an empty request hook factory")
  {
    mse::RequestHookFactory::GetInstance().Clear();

    WHEN("a new hook is registered")
    {
      mse::RequestHookFactory::GetInstance().Register<TestRequestHook::Parameters>(TestRequestHook::Create);

      THEN("the factory can be used to create the corresponding hook from an rValue")
      {
        std::unique_ptr<mse::RequestHook> hook =
            mse::RequestHookFactory::GetInstance().Create(TestRequestHook::Parameters{1, 2});
        REQUIRE(hook != nullptr);
      }
      THEN("the factory can be used to create the corresponding hook from an lValue")
      {
        TestRequestHook::Parameters params{1, 2};
        std::unique_ptr<mse::RequestHook> hook = mse::RequestHookFactory::GetInstance().Create(params);
        REQUIRE(hook != nullptr);
      }
      THEN("the factory can be used to create the corresponding hook from a move reference")
      {
        TestRequestHook::Parameters params{1, 2};
        std::unique_ptr<mse::RequestHook> hook = mse::RequestHookFactory::GetInstance().Create(std::move(params));
        REQUIRE(hook != nullptr);
      }
      THEN("the factory can be used to create the corresponding hook from a const value")
      {
        const TestRequestHook::Parameters params{1, 2};
        std::unique_ptr<mse::RequestHook> hook = mse::RequestHookFactory::GetInstance().Create(params);
        REQUIRE(hook != nullptr);
      }

      AND_WHEN("a second hook is created with the same parameter type")
      {
        THEN("an invalid argument excetion is thrown")
        {
          REQUIRE_THROWS_AS(
              mse::RequestHookFactory::GetInstance().Register<TestRequestHook::Parameters>(TestRequestHook::Create),
              std::invalid_argument);
        }
      }

      mse::RequestHookFactory::GetInstance().Clear();
    }

    WHEN("an unknown hook is created")
    {
      class UnknownParameters
      {
      };
      THEN("an out of range exception is thrown")
      {
        REQUIRE_THROWS_AS(mse::RequestHookFactory::GetInstance().Create(UnknownParameters()), std::out_of_range);
      }
    }
  }
}
