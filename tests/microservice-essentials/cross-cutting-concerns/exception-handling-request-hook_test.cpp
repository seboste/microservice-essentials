#include <catch2/catch_test_macros.hpp>
#include <microservice-essentials/cross-cutting-concerns/exception-handling-request-hook.h>
#include <utilities/history_logger.h>

SCENARIO("Exception Handling Request Hook Creation", "[cross-cutting-concerns][exception handling][request-hook]")
{    
    WHEN("a exception handling request hook is created based on the parameters")
    {
        std::unique_ptr<mse::RequestHook> exception_handling_request_hook = mse::RequestHookFactory::GetInstance().Create(mse::ExceptionHandlingRequestHook::Parameters());
        THEN("the request hook is not null")
        {
            REQUIRE(exception_handling_request_hook != nullptr);
        }       
    }
}

SCENARIO( "Exception Handling Predicate", "[cross-cutting-concerns][exception handling]" )
{
    GIVEN("Exception Type Predicate for specific type")
    {
        mse::ExceptionHandling::ExceptionOfType<std::runtime_error> runtime_error_predicate;
        WHEN("the specific type is tested")
        {
            bool test_result = runtime_error_predicate.Test(std::make_exception_ptr(std::runtime_error("runtime error")));
            THEN("test result is true")
            {
                REQUIRE(test_result == true);
            }
        }
        WHEN("a derived type is tested")
        {
            bool test_result = runtime_error_predicate.Test(std::make_exception_ptr(std::range_error("range error")));
            THEN("test result is true")
            {
                REQUIRE(test_result == true);
            }
        }
        WHEN("base exception type is tested")
        {
            bool test_result = runtime_error_predicate.Test(std::make_exception_ptr(std::exception()));
            THEN("test result is false")
            {
                REQUIRE(test_result == false);
            }
        }
        WHEN("a completely unrelated type is tested")
        {
            bool test_result = runtime_error_predicate.Test(std::make_exception_ptr(10));
            THEN("test result is false")
            {
                REQUIRE(test_result == false);
            }
        }
    }
    GIVEN("AnyException Predicate")
    {
        mse::ExceptionHandling::AnyException any_exception_predicate;
        WHEN("some random exception is tested")
        {
            bool test_result = any_exception_predicate.Test(std::make_exception_ptr(std::bad_alloc()));
            THEN("test result is true")
            {
                REQUIRE(test_result == true);
            }
        }
    }
    WHEN("Factory Method is used to create Exception Type Predicate for specific type")
    {
        std::shared_ptr<mse::ExceptionHandling::ExceptionTypePredicate> predicate = mse::ExceptionHandling::Is<mse::ExceptionHandling::ExceptionOfType<std::string>>();
        THEN("predicate is not null")
        {
            REQUIRE(predicate != nullptr);
        }
    }
}

SCENARIO( "Exception Handling Request Hook", "[cross-cutting-concerns][exception handling][request-hook]" )
{
    GIVEN("Exception Handling Request Hook with single exception handling definition")
    {
        mse::ExceptionHandlingRequestHook request_hook(mse::ExceptionHandlingRequestHook::Parameters(
            {
                { mse::ExceptionHandling::Is<mse::ExceptionHandling::ExceptionOfType<std::runtime_error>>(), mse::Status{mse::StatusCode::data_loss, "test"}, mse::LogLevel::invalid, false }
            }));
        
        WHEN("the registered exception is thrown")
        {
            mse::Context context;
            mse::Status status = request_hook.Process([](mse::Context&)
            {
                throw std::runtime_error("some runtime error");
                return mse::Status::OK;
            }
            , context);
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
                REQUIRE_THROWS_AS(
                    request_hook.Process([](mse::Context&)
                    {
                        throw std::logic_error("some logic error");
                        return mse::Status::OK;
                    }
                    , context)
                    , std::logic_error);
            };                    
        }
        WHEN("some error is returned")
        {
            mse::Context context;
            mse::Status status = request_hook.Process([](mse::Context&)
            {                
                return mse::Status{ mse::StatusCode::not_found, "test"};
            }
            , context);
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
            mse::Status status = request_hook.Process([](mse::Context&)
            {
                throw std::invalid_argument("some exception details");
                return mse::Status::OK;
            }
            , context);
            THEN("invalid_argument status is returned")
            {
                REQUIRE(status.code == mse::StatusCode::invalid_argument);
                REQUIRE(status.details == "invalid argument exception: some exception details");
            }
        }
        WHEN("out of range excption is thrown")
        {
            mse::Context context;
            mse::Status status = request_hook.Process([](mse::Context&)
            {
                throw std::out_of_range("some exception details");
                return mse::Status::OK;
            }
            , context);
            THEN("invalid_argument status is returned")
            {
                REQUIRE(status.code == mse::StatusCode::out_of_range);
                REQUIRE(status.details == "out of range argument exception: some exception details");
            }
        }
        WHEN("some other exception is thrown")
        {
            mse::Context context;
            mse::Status status = request_hook.Process([](mse::Context&)
            {
                throw std::string("bla");
                return mse::Status::OK;
            }
            , context);
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
        mse::ExceptionHandlingRequestHook request_hook(mse::ExceptionHandlingRequestHook::Parameters(
            {
                { Is<ExceptionOfType<std::logic_error>>()  , mse::Status::OK, mse::LogLevel::invalid, false },  //no logging
                { Is<ExceptionOfType<std::runtime_error>>(), mse::Status::OK, mse::LogLevel::trace  , false }   //logging                
            }));        

        WHEN("non logging exception is thrown")
        {            
            request_hook.Process([](mse::Context&)
            {
                throw std::logic_error("my custom exception details");
                return mse::Status::OK;
            }
            , context);

            THEN("log does not contain my custom exception details")
            {
                REQUIRE(log._log_history[mse::LogLevel::trace].empty());                
            }
        }
        WHEN("logging exception is thrown")
        {            
            request_hook.Process([](mse::Context&)
            {
                throw std::runtime_error("my custom exception details");
                return mse::Status::OK;
            }
            , context);

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
        mse::ExceptionHandlingRequestHook request_hook(mse::ExceptionHandlingRequestHook::Parameters(
            {
                { Is<ExceptionOfType<std::logic_error>>()  , mse::Status::OK, mse::LogLevel::invalid, true },  //detail forwarding
                { Is<ExceptionOfType<std::runtime_error>>(), mse::Status::OK, mse::LogLevel::invalid, false }  //no detail forwarding
            }));        

        WHEN("non forwarding exception is thrown")
        {            
            mse::Status status = request_hook.Process([](mse::Context&)
            {
                throw std::runtime_error("my custom exception details");
                return mse::Status::OK;
            }
            , context);

            THEN("details do not contain my custom exception details")
            {
                std::cout << "details: " << status.details << std::endl;
                REQUIRE(status.details.find("my custom exception details") == std::string::npos);
            }
        }
        WHEN("forwarding exception is thrown")
        {            
            mse::Status status = request_hook.Process([](mse::Context&)
            {
                throw std::logic_error("my custom exception details");
                return mse::Status::OK;
            }
            , context);

            THEN("details contain my custom exception details")
            {
                REQUIRE(status.details.find("my custom exception details") != std::string::npos);
            }
        }
    }
}
