#include <catch2/catch_test_macros.hpp>
#include <microservice-essentials/request/request-decorator.h>

namespace 
{

class TestRequestDecorator : public mse::RequestDecorator
{
public:
    TestRequestDecorator(mse::StatusCode preprocess_status_code, mse::StatusCode postprocess_status_code)
        : mse::RequestDecorator("test")
        , _preprocess_status_code(preprocess_status_code)
        , _postprocess_status_code(postprocess_status_code)
        {
        }

    virtual mse::Status pre_process(mse::Context& context) override
    {
        context.Insert({{"pre", "process"}});
        _call_history.push_back(CallType::PRE_PROCESS);
        return mse::Status{_preprocess_status_code};
    }

    virtual mse::Status post_process(mse::Context& context) override
    {
        context.Insert({{"post", "process"}});
        _call_history.push_back(CallType::POST_PROCESS);
        return mse::Status{_postprocess_status_code};
    }

    enum class CallType
    {
        PRE_PROCESS,
        POST_PROCESS,
        WRAPPED_FUNCTION
    };

    std::vector<CallType> _call_history;


private:
    mse::StatusCode _preprocess_status_code;
    mse::StatusCode _postprocess_status_code;
};


}


SCENARIO("RequestDecorator", "[request]")
{
    GIVEN("a test request decorator that returns success")
    {
        TestRequestDecorator test_request_decorator(mse::StatusCode::ok, mse::StatusCode::ok);

        WHEN("process is called on a successful function")
        {
            mse::Context context({{"test", "123"}});
            mse::Status status = test_request_decorator.Process([&test_request_decorator](mse::Context& context) 
            {
                REQUIRE(context.Contains("test"));
                REQUIRE(context.Contains("pre"));
                REQUIRE(!context.Contains("post"));

                context.Insert({{"func", "xyz"}});

                test_request_decorator._call_history.push_back(TestRequestDecorator::CallType::WRAPPED_FUNCTION);
                return mse::Status{mse::StatusCode::ok};
            }, context);

            REQUIRE(status);
            
            THEN("pre process has been called")
            {
                REQUIRE(test_request_decorator._call_history[0] == TestRequestDecorator::CallType::PRE_PROCESS);
                AND_THEN("the function has been called")
                {
                    REQUIRE(test_request_decorator._call_history[1] == TestRequestDecorator::CallType::WRAPPED_FUNCTION);
                    AND_THEN("post process has been called")
                    {
                        REQUIRE(test_request_decorator._call_history[2] == TestRequestDecorator::CallType::POST_PROCESS);
                    }
                }
            }
            THEN("meta data is correct")
            {
                REQUIRE(context.Contains("test"));
                REQUIRE(context.Contains("pre"));
                REQUIRE(context.Contains("post"));
                REQUIRE(context.Contains("func"));
            }
        }
    }
}

