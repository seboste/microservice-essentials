#include <catch2/catch_test_macros.hpp>
#include <microservice-essentials/request/request-processor.h>
#include <microservice-essentials/observability/logger.h>


namespace
{
class DummyRequestHook : public mse::RequestHook
{
public:
    DummyRequestHook(const std::string& name, mse::StatusCode preprocess_status_code, mse::StatusCode postprocess_status_code)
        : mse::RequestHook(name)
        , _preprocess_status_code(preprocess_status_code)
        , _postprocess_status_code(postprocess_status_code)
    {
    }

    virtual mse::Status pre_process(mse::Context& context) override
    {
        return mse::Status{_preprocess_status_code};
    }

    virtual mse::Status post_process(mse::Context& context) override
    {
        return mse::Status{_postprocess_status_code};
    }

private:
    mse::StatusCode _preprocess_status_code;
    mse::StatusCode _postprocess_status_code;
};


}

SCENARIO("RequestProcessor", "[request]")
{
    mse::ConsoleLogger logger(mse::LogLevel::trace);
    
    GIVEN("a request processor")
    {
        mse::RequestProcessor processor("test", mse::Context());
        WHEN("two successful hooks are added")
        {
            processor
                .With(std::make_unique<DummyRequestHook>("a", mse::StatusCode::ok, mse::StatusCode::ok))
                .With(std::make_unique<DummyRequestHook>("b", mse::StatusCode::ok, mse::StatusCode::ok));            

            AND_WHEN("some function is processed")
            {
                mse::Status status = processor.Process([](mse::Context& ctx)
                    { 
                        MSE_LOG_TRACE("function is executed");
                        return mse::Status{mse::StatusCode::ok};
                    });

                THEN("execution has been successful")
                {
                    REQUIRE(status);
                }
            }
        }        
    }
}