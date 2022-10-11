#include <catch2/catch_test_macros.hpp>
#include <microservice-essentials/request/request-processor.h>
#include <microservice-essentials/request/request-hook-factory.h>
#include <microservice-essentials/observability/logger.h>


namespace
{
class DummyRequestHook : public mse::RequestHook
{
public:

    typedef std::vector<std::pair<std::string, std::string>> CallHistory; //hook name, function name

    struct Parameters
    {
        std::string name;        
        mse::StatusCode preprocess_status_code;
        mse::StatusCode postprocess_status_code;
        CallHistory& call_history;
    };

    static std::unique_ptr<mse::RequestHook> Create(std::any parameters)
    {
        Parameters params = std::any_cast<Parameters>(parameters);
        return std::make_unique<DummyRequestHook>(params.name, params.preprocess_status_code, params.postprocess_status_code, params.call_history);
    }

    DummyRequestHook(const std::string& name, mse::StatusCode preprocess_status_code, mse::StatusCode postprocess_status_code, CallHistory& call_history)
        : mse::RequestHook(name)
        , _preprocess_status_code(preprocess_status_code)
        , _postprocess_status_code(postprocess_status_code)
        , _call_history(call_history)
    {
    }

    virtual mse::Status pre_process(mse::Context& context) override
    {
        _call_history.push_back({_name, "pre"});
        return mse::Status{_preprocess_status_code};
    }

    virtual mse::Status post_process(mse::Context& context, mse::Status status) override
    {
        _call_history.push_back({_name, "post"});
        return status
            ? mse::Status{_postprocess_status_code}
            : status;
    }

private:
    mse::StatusCode _preprocess_status_code;
    mse::StatusCode _postprocess_status_code;
    CallHistory& _call_history;
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
            DummyRequestHook::CallHistory call_history;

            processor
                .With(std::make_unique<DummyRequestHook>("a", mse::StatusCode::ok, mse::StatusCode::ok, call_history))
                .With(std::make_unique<DummyRequestHook>("b", mse::StatusCode::ok, mse::StatusCode::ok, call_history));

            AND_WHEN("some function is processed")
            {                
                mse::Status status = processor.Process([&call_history](mse::Context& ctx)
                    { 
                        MSE_LOG_TRACE("function is executed");
                        call_history.push_back({"func", "func"});
                        return mse::Status{mse::StatusCode::ok};
                    });

                THEN("execution has been successful")
                {
                    REQUIRE(status);
                }
                AND_THEN("the execution order is correct")
                {
                    REQUIRE(call_history.size() == 5);
                    REQUIRE((call_history[0].first == "a" && call_history[0].second == "pre"));
                    REQUIRE((call_history[1].first == "b" && call_history[1].second == "pre"));
                    REQUIRE((call_history[2].first == "func" && call_history[2].second == "func"));
                    REQUIRE((call_history[3].first == "b" && call_history[3].second == "post"));
                    REQUIRE((call_history[4].first == "a" && call_history[4].second == "post"));                    
                }
            }
        }
        AND_GIVEN("a factory registration for the DummyRequestHook")
        {
            mse::RequestHookFactory::GetInstance().Clear();
            mse::RequestHookFactory::GetInstance().Register<DummyRequestHook::Parameters>(DummyRequestHook::Create);
            WHEN("a parameter based hook is added") 
            {
                DummyRequestHook::CallHistory call_history;
                processor.With(DummyRequestHook::Parameters({"dummy_param", mse::StatusCode::ok, mse::StatusCode::ok, call_history }));

                AND_WHEN("some function is processed")
                {
                     mse::Status status = processor.Process([&call_history](mse::Context& ctx)
                    { 
                        MSE_LOG_TRACE("function is executed");
                        call_history.push_back({"func", "func"});
                        return mse::Status{mse::StatusCode::ok};
                    });

                    THEN("execution has been successful")
                    {
                        REQUIRE(status);
                    }
                    AND_THEN("the execution order is correct")
                    {
                        REQUIRE(call_history.size() == 3);
                        REQUIRE((call_history[0].first == "dummy_param" && call_history[0].second == "pre"));                    
                        REQUIRE((call_history[1].first == "func" && call_history[1].second == "func"));
                        REQUIRE((call_history[2].first == "dummy_param" && call_history[2].second == "post"));                               
                    }
                }
            }
        }
    }
}