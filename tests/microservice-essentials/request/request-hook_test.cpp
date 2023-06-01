#include <catch2/catch_test_macros.hpp>
#include <microservice-essentials/request/request-hook.h>

namespace
{

class TestRequestHook : public mse::RequestHook
{
public:
  TestRequestHook(mse::StatusCode preprocess_status_code, mse::StatusCode postprocess_status_code)
      : mse::RequestHook("test"), _preprocess_status_code(preprocess_status_code),
        _postprocess_status_code(postprocess_status_code)
  {
  }

  virtual mse::Status pre_process(mse::Context& context) override
  {
    context.Insert({{"pre", "process"}});
    _call_history.push_back(CallType::PRE_PROCESS);
    return mse::Status{_preprocess_status_code, ""};
  }

  virtual mse::Status post_process(mse::Context& context, mse::Status status) override
  {
    context.Insert({{"post", "process"}});
    _call_history.push_back(CallType::POST_PROCESS);
    return status ? mse::Status{_postprocess_status_code, ""} : status;
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

} // namespace

SCENARIO("RequestHook", "[request]")
{
  GIVEN("a test request hook that returns success")
  {
    TestRequestHook test_request_hook(mse::StatusCode::ok, mse::StatusCode::ok);

    WHEN("process is called on a successful function")
    {
      mse::Context context({{"test", "123"}});
      mse::Status status = test_request_hook.Process(
          [&test_request_hook](mse::Context& context) {
            REQUIRE(context.Contains("test"));
            REQUIRE(context.Contains("pre"));
            REQUIRE(!context.Contains("post"));

            context.Insert({{"func", "xyz"}});

            test_request_hook._call_history.push_back(TestRequestHook::CallType::WRAPPED_FUNCTION);
            return mse::Status{mse::StatusCode::ok, ""};
          },
          context);

      REQUIRE(status);

      THEN("pre process has been called")
      {
        REQUIRE(test_request_hook._call_history[0] == TestRequestHook::CallType::PRE_PROCESS);
        AND_THEN("the function has been called")
        {
          REQUIRE(test_request_hook._call_history[1] == TestRequestHook::CallType::WRAPPED_FUNCTION);
          AND_THEN("post process has been called")
          {
            REQUIRE(test_request_hook._call_history[2] == TestRequestHook::CallType::POST_PROCESS);
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

  GIVEN("a test request hook that returns an error in preprocess")
  {
    TestRequestHook test_request_hook(mse::StatusCode::aborted, mse::StatusCode::ok);

    WHEN("process is called on a successful function")
    {
      mse::Context context({{"test", "123"}});
      mse::Status status = test_request_hook.Process(
          [&test_request_hook](mse::Context& context) {
            REQUIRE(context.Contains("test"));
            REQUIRE(context.Contains("pre"));
            REQUIRE(!context.Contains("post"));

            context.Insert({{"func", "xyz"}});

            test_request_hook._call_history.push_back(TestRequestHook::CallType::WRAPPED_FUNCTION);
            return mse::Status{mse::StatusCode::ok, ""};
          },
          context);

      REQUIRE(status.code == mse::StatusCode::aborted);

      THEN("only pre process has been called")
      {
        REQUIRE(test_request_hook._call_history.size() == 1);
        REQUIRE(test_request_hook._call_history[0] == TestRequestHook::CallType::PRE_PROCESS);
      }
      THEN("meta data is correct")
      {
        REQUIRE(context.Contains("test"));
        REQUIRE(context.Contains("pre"));
        REQUIRE(!context.Contains("post"));
        REQUIRE(!context.Contains("func"));
      }
    }
  }

  GIVEN("a test request hook that returns an error in postprocess")
  {
    TestRequestHook test_request_hook(mse::StatusCode::ok, mse::StatusCode::aborted);

    WHEN("process is called on a successful function")
    {
      mse::Context context({{"test", "123"}});
      mse::Status status = test_request_hook.Process(
          [&test_request_hook](mse::Context& context) {
            REQUIRE(context.Contains("test"));
            REQUIRE(context.Contains("pre"));
            REQUIRE(!context.Contains("post"));

            context.Insert({{"func", "xyz"}});

            test_request_hook._call_history.push_back(TestRequestHook::CallType::WRAPPED_FUNCTION);
            return mse::Status{mse::StatusCode::ok, ""};
          },
          context);

      REQUIRE(status.code == mse::StatusCode::aborted);

      THEN("pre process has been called")
      {
        REQUIRE(test_request_hook._call_history[0] == TestRequestHook::CallType::PRE_PROCESS);
        AND_THEN("the function has been called")
        {
          REQUIRE(test_request_hook._call_history[1] == TestRequestHook::CallType::WRAPPED_FUNCTION);
          AND_THEN("post process has been called")
          {
            REQUIRE(test_request_hook._call_history[2] == TestRequestHook::CallType::POST_PROCESS);
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

  GIVEN("a test request hook that returns success")
  {
    TestRequestHook test_request_hook(mse::StatusCode::ok, mse::StatusCode::ok);

    WHEN("process is called on a function that returns an error")
    {
      mse::Context context({{"test", "123"}});
      mse::Status status = test_request_hook.Process(
          [&test_request_hook](mse::Context& context) {
            REQUIRE(context.Contains("test"));
            REQUIRE(context.Contains("pre"));
            REQUIRE(!context.Contains("post"));

            context.Insert({{"func", "xyz"}});

            test_request_hook._call_history.push_back(TestRequestHook::CallType::WRAPPED_FUNCTION);
            return mse::Status{mse::StatusCode::aborted, ""};
          },
          context);

      REQUIRE(status.code == mse::StatusCode::aborted);

      THEN("pre process has been called")
      {
        REQUIRE(test_request_hook._call_history[0] == TestRequestHook::CallType::PRE_PROCESS);
        AND_THEN("the function has been called")
        {
          REQUIRE(test_request_hook._call_history[1] == TestRequestHook::CallType::WRAPPED_FUNCTION);
          AND_THEN("post process has been called")
          {
            REQUIRE(test_request_hook._call_history[2] == TestRequestHook::CallType::POST_PROCESS);
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
