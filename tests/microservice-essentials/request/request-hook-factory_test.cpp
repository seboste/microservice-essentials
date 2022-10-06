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

    TestRequestHook(const Parameters& parameters)
        : mse::RequestHook("test")
        {
        }    
   
    static std::unique_ptr<mse::RequestHook> Create(const std::any& parameters)
    {
        return std::make_unique<TestRequestHook>(std::any_cast<Parameters>(parameters));
    }

private:
    
};


}


SCENARIO("RequestHookFactory", "[request]")
{
    GIVEN("an empty request hook factory")
    {
        mse::RequestHookFactory::GetInstance().Clear();

        WHEN("a new hook is registered")
        {
            mse::RequestHookFactory::GetInstance().Register<TestRequestHook::Parameters>(TestRequestHook::Create);

            THEN("the factory can be used to create the corresponding hook")
            {
                std::unique_ptr<mse::RequestHook> hook = mse::RequestHookFactory::GetInstance().Create(TestRequestHook::Parameters{1,2});
                REQUIRE(hook != nullptr);
            }
        }
    }
}