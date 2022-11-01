#include <catch2/catch_test_macros.hpp>
#include <microservice-essentials/cross-cutting-concerns/error-forwarding-request-hook.h>
#include <microservice-essentials/cross-cutting-concerns/exception-handling-request-hook.h>
#include <utilities/history_logger.h>

SCENARIO("Error Forwarding Request Hook Creation", "[cross-cutting-concerns][error forwarding][request-hook]")
{    
    WHEN("a error forwarding request hook is created based on the parameters")
    {
        std::unique_ptr<mse::RequestHook> error_forwarding_request_hook = mse::RequestHookFactory::GetInstance().Create(mse::ErrorForwardingRequestHook::Parameters());
        THEN("the request hook is not null")
        {
            REQUIRE(error_forwarding_request_hook != nullptr);
        }       
    }

    WHEN("all error codes are included")
    {
        mse::ErrorForwardingRequestHook::Parameters parameters;
        parameters.IncludeAllErrorCodes( mse::Status{ mse::StatusCode::cancelled, "test"} );

        THEN("The Ok Status is not included")
        {
            REQUIRE(parameters.status_code_mapping.find(mse::StatusCode::ok) == parameters.status_code_mapping.end());
        }
        THEN("code mapping count matches the number of error codes")
        {
            REQUIRE(parameters.status_code_mapping.size() == 
            (static_cast<int>(mse::StatusCode::highest) - static_cast<int>(mse::StatusCode::lowest) + 1 //total number of codes
            - 1 //subtract the ok status
            ));
        }
        THEN("some random entry has the defined status")
        {
            REQUIRE(parameters.status_code_mapping[mse::StatusCode::internal].code == mse::StatusCode::cancelled);
            REQUIRE(parameters.status_code_mapping[mse::StatusCode::internal].details == "test");
        }


        AND_WHEN("a specific error code is excluded")
        {
            size_t status_code_map_size = parameters.status_code_mapping.size();
            parameters.Exclude(mse::StatusCode::internal);
            THEN("code mapping count is reduced by one")
            {
                REQUIRE(parameters.status_code_mapping.size() == (status_code_map_size - 1));
            }
            THEN("specific error code is missing")
            {
                REQUIRE(parameters.status_code_mapping.find(mse::StatusCode::internal) == parameters.status_code_mapping.end());
            }
        }

        AND_WHEN("ok status code is included")
        {
            size_t status_code_map_size = parameters.status_code_mapping.size();
            parameters.Include(mse::StatusCode::ok, mse::Status{ mse::StatusCode::data_loss, "ok" });            
            THEN("code mapping count is increased by one")
            {
                REQUIRE(parameters.status_code_mapping.size() == (status_code_map_size + 1));
            }
            
            auto cit = parameters.status_code_mapping.find(mse::StatusCode::ok);
            THEN("ok error code is available")
            {
                REQUIRE(cit != parameters.status_code_mapping.end());

                AND_THEN("ok error code is mapped to defined status")
                {
                    REQUIRE(cit->second.code == mse::StatusCode::data_loss);
                    REQUIRE(cit->second.details == "ok");
                }
            }
        }
    }
}

SCENARIO("Error Forwarding Exception", "[cross-cutting-concerns][error forwarding][request-hook]")
{
    WHEN("an error forwarding exception is created")
    {
        mse::ErrorForwardingException e(mse::Status{ mse::StatusCode::unauthenticated, "unauthenticated"}, "some exception details");
        THEN("status is set correctly")
        {
            REQUIRE(e.GetStatus().code == mse::StatusCode::unauthenticated);
            REQUIRE(e.GetStatus().details == "unauthenticated");
        }
        THEN("exception details are correct")
        {
            REQUIRE(std::string(e.what()) == "some exception details");
        }
    }
}

SCENARIO("Error Forwarding Request Hook", "[cross-cutting-concerns][error forwarding][request-hook]")
{
    GIVEN("an error forwarding request hook")
    {
        mse::ErrorForwardingRequestHook hook(mse::ErrorForwardingRequestHook::Parameters().Include(
                mse::StatusCode::already_exists, mse::Status{ mse::StatusCode::unavailable, "unavailable"}
            ));
        mse::Context context({{"request", "test"}});

        WHEN("process is called on a function that returns ok")
        {
            mse::Status status = hook.Process([](const mse::Context&){ return mse::Status::OK; }, context);
            THEN("returned status is ok")
            {
                REQUIRE(status.code == mse::StatusCode::ok);
            }
        }
        WHEN("process is called on a function that returns the registered status code")
        {
            THEN("an exception of type error forwarding exception is thrown")
            {                
                try
                {
                    hook.Process([](const mse::Context&) { return mse::Status{ mse::StatusCode::already_exists, "already exists"}; }, context);
                    REQUIRE(false);
                }
                catch(const mse::ErrorForwardingException& e)
                {
                    THEN("status is unavailable")
                    {
                        REQUIRE(e.GetStatus().code == mse::StatusCode::unavailable);
                        REQUIRE(e.GetStatus().details == "unavailable");
                    }
                    THEN("details contain the original error details")
                    {
                        REQUIRE(std::string(e.what()).find("already exists") != std::string::npos);
                    }                    
                }
                catch(...)
                {
                    REQUIRE(false);
                }
            }
        }        
    }

    GIVEN("a default exception handling hook")
    {
        mse::ExceptionHandlingRequestHook exception_handling_hook{mse::ExceptionHandlingRequestHook::Parameters()};
        
        AND_GIVEN("a function that calls process of a error forwarding request hook throwing an exception")
        {
            auto f = [](mse::Context& context)->mse::Status
            {
                mse::ErrorForwardingRequestHook error_forwarder(mse::ErrorForwardingRequestHook::Parameters().IncludeAllErrorCodes());
                return error_forwarder.Process([](mse::Context&) { return mse::Status{ mse::StatusCode::invalid_argument, "invalid argument" }; } , context);
            };

            WHEN("the exception handling hook's process method is called on exception forwarder function")
            {
                mse::Context context;
                mse::Status status;
                THEN("no exception is thrown")
                {
                    REQUIRE_NOTHROW(status = exception_handling_hook.Process(f, context));
                    
                    AND_THEN("status is unavailable")
                    {
                        REQUIRE(status.code == mse::StatusCode::unavailable);
                    }

                    AND_THEN("error details are not leaked to the outside")
                    {
                        REQUIRE(status.details.find("invalid argument") == std::string::npos);
                    }
                }                
            }
        }
    }
}