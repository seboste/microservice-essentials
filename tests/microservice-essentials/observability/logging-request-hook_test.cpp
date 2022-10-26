#include <catch2/catch_test_macros.hpp>
#include <microservice-essentials/observability/logging-request-hook.h>
#include <microservice-essentials/request/request-hook-factory.h>
#include <utilities/history_logger.h>


SCENARIO( "LoggingRequestHook", "[observability][logging][request-hook]" )
{
    WHEN("the request hook factory is used to create a LoggingRequestHook")
    {
        std::unique_ptr<mse::RequestHook> request_hook = mse::RequestHookFactory::GetInstance().Create(mse::LoggingRequestHook::Parameters{});
        THEN("the request hook is not null")
        {
            REQUIRE(request_hook != nullptr);
        }
        THEN("the request hook is of type LoggingRequestHook")
        {
            REQUIRE(dynamic_cast<mse::LoggingRequestHook*>(request_hook.get()) != nullptr);
        }
    }

    GIVEN("a logging request hook")
    {
        mse::LoggingRequestHook hook;

        WHEN("the process method is called on a successful function")
        {
            bool has_been_called = false;
            mse::Context context;
            mse::Status status = hook.Process([&](mse::Context&)
                {
                    has_been_called = true;
                    return mse::Status::OK;
                }, context
            );

            THEN("the status is ok")
            {
                REQUIRE(status);
            }
            THEN("the function has been called")
            {
                REQUIRE(has_been_called == true);
            }
        }
    }

    GIVEN("a history logger")
    {
        mse_test::HistoryLogger log;
        AND_GIVEN("a logging request hook with different log levels")
        {
            mse::LoggingRequestHook hook({mse::LogLevel::info, mse::LogLevel::warn});
            mse::Context context;

            WHEN("the process method is called on a successful function")
            {
                hook.Process([&](mse::Context&) { return mse::Status::OK; }, context);
            }


            AND_GIVEN("context with request set to test")
            {
                context.Insert("request", "test");
                WHEN("request type is set to incoming")
                {
                    hook.SetRequestType(mse::RequestType::incoming);
                    AND_WHEN("the process method is called on a successful function")
                    {
                        hook.Process([&](mse::Context&) { return mse::Status::OK; }, context);
                        THEN("there are 2 messages in the first category")
                        {
                            REQUIRE(log._log_history[mse::LogLevel::info].size() == 2);
                        
                            THEN("first message contains 'handling' and 'test'")
                            {
                                REQUIRE(log._log_history[mse::LogLevel::info][0].find("handling") != std::string::npos);
                                REQUIRE(log._log_history[mse::LogLevel::info][0].find("test") != std::string::npos);
                            }
                            THEN("second message contains 'handled', 'test', and 'OK'")
                            {
                                REQUIRE(log._log_history[mse::LogLevel::info][1].find("handled") != std::string::npos);
                                REQUIRE(log._log_history[mse::LogLevel::info][1].find("test") != std::string::npos);
                                REQUIRE(log._log_history[mse::LogLevel::info][1].find("OK") != std::string::npos);
                            }
                        }
                    }
                }
                WHEN("request type is set to outgoing")
                {
                    hook.SetRequestType(mse::RequestType::outgoing);
                    AND_WHEN("the process method is called on a successful function")
                    {
                        hook.Process([&](mse::Context&) { return mse::Status({mse::StatusCode::not_found, ""}); }, context);
                        THEN("there is 1 message in the first and 1 message in the second category")
                        {
                            REQUIRE(log._log_history[mse::LogLevel::info].size() == 1);
                            REQUIRE(log._log_history[mse::LogLevel::warn].size() == 1);
                        
                            AND_THEN("first message contains 'issuing' and 'test'")
                            {
                                REQUIRE(log._log_history[mse::LogLevel::info][0].find("issuing") != std::string::npos);
                                REQUIRE(log._log_history[mse::LogLevel::info][0].find("test") != std::string::npos);
                            }
                            AND_THEN("second message contains 'issued', 'test', and 'NOT_FOUND'")
                            {
                                REQUIRE(log._log_history[mse::LogLevel::warn][0].find("issued") != std::string::npos);
                                REQUIRE(log._log_history[mse::LogLevel::warn][0].find("test") != std::string::npos);
                                REQUIRE(log._log_history[mse::LogLevel::warn][0].find("NOT_FOUND") != std::string::npos);
                            }
                        }
                    }
                }
            }
        }
    }
}