#include <catch2/catch_test_macros.hpp>
#include <microservice-essentials/status.h>
#include <iostream>
#include <memory>
#include <sstream>

SCENARIO( "Status", "[status]" )
{
    WHEN("some status with default values is created")
    {
        mse::Status status;
        THEN("details are empty")
        {
            REQUIRE(status.details == "");
        }
        AND_THEN("status_code is ok")
        {
            REQUIRE(status.code == mse::StatusCode::ok);
        }
        AND_THEN("status is ok")
        {
            REQUIRE(status == true);
        }
    }

    WHEN("some status is construced with {} operator")
    {
        mse::Status status { mse::StatusCode::unimplemented , "test"};
        THEN("details and status code are set")
        {
            REQUIRE(status.details == "test");        
            REQUIRE(status.code == mse::StatusCode::unimplemented);
        }

        AND_THEN("status is not ok")
        {
            REQUIRE(status == false);
        }
    }
}

SCENARIO( "Status Code", "[status]" )
{
    const std::vector<std::string> status_codes = {"OK" ,"CANCELLED" ,"UNKNOWN" ,"INVALID_ARGUMENT" ,"DEADLINE_EXCEEDED" ,"NOT_FOUND" ,"ALREADY_EXISTS" ,"PERMISSION_DENIED" ,"RESOURCE_EXHAUSTED" ,"FAILED_PRECONDITION" ,"ABORTED" ,"OUT_OF_RANGE" ,"UNIMPLEMENTED" ,"INTERNAL" ,"UNAVAILABLE" ,"DATA_LOSS" ,"UNAUTHENTICATED" };
    int code_number = 0;
    for(auto status_code_string : status_codes)
    {
        GIVEN(std::string("the status code ") + status_code_string)
        {
            std::stringstream status_code_stream(status_code_string);
            WHEN("it is converted to a status_cde")
            {
                mse::StatusCode status_code = mse::StatusCode::invalid;
                mse::from_string(status_code_string, status_code);
                THEN("the status code is not invalid")
                {
                    REQUIRE(status_code != mse::StatusCode::invalid);
                }
                AND_THEN("the level number is increasing")
                {
                    REQUIRE(static_cast<int>(status_code) == code_number);
                }
                AND_WHEN("the status code is converted to a string")
                {
                    std::string converted_status_code_string = mse::to_string(status_code);
                    THEN("the string is identical to the original code string")
                    {
                        REQUIRE(converted_status_code_string == status_code_string);
                    }
                }
                AND_WHEN("the >> operator is used to do the conversion")
                {
                    mse::StatusCode status_code_from_stream = mse::StatusCode::invalid;
                    bool result = (status_code_stream >> status_code_from_stream);
                    THEN("the conversion succeeds")
                    {
                        REQUIRE(result == true);
                    }
                    AND_THEN("it is identical to the normal conversion")
                    {
                        REQUIRE(status_code_from_stream == status_code);
                    }
                }
                AND_WHEN("the << operator is used to write the status code into a stream")
                {
                    std::stringstream stream;
                    std::ostream& result = (stream << status_code);
                    THEN("the return value refers to the input stream")
                    {
                        REQUIRE(&result == static_cast<std::ostream*>(&stream));
                    }
                    AND_THEN("the stream consists of the original status code string")
                    {
                        REQUIRE(stream.str() == status_code_string);
                    }
                }
            }            
        }        
        code_number++;
    }
    
    GIVEN("an invalid status code string")
    {
        const std::string invalid_status_code_string = "BLA";
        std::stringstream invalid_status_code_stream(invalid_status_code_string);
        WHEN("it is converted to a status_code")
        {
            mse::StatusCode status_code = mse::StatusCode::internal;
            mse::from_string(invalid_status_code_string, status_code);
            THEN("the status code is invalid")
            {
                REQUIRE(status_code == mse::StatusCode::invalid);
            }        
        }
        WHEN("the >> operator is used to do the conversion")
        {
            mse::StatusCode status_code_from_stream = mse::StatusCode::internal;
            bool result = (invalid_status_code_stream >> status_code_from_stream);
            THEN("the conversion fails")
            {
                REQUIRE(result == false);
            }
            AND_THEN("the converted status code is invalid")
            {
                REQUIRE(status_code_from_stream == mse::StatusCode::invalid);
            }
        }
    }
    GIVEN("an invalid status code")
    {
        WHEN("it is converted to a string")
        {
            THEN("an invalid argument exception is thrown")
            {
                REQUIRE_THROWS_AS(mse::to_string(mse::StatusCode::invalid), std::invalid_argument);
            }
        }
        WHEN("it is put into a stream using the << operator")
        {
            std::stringstream stream;
            THEN("an invalid argument exception is thrown")
            {
                REQUIRE_THROWS_AS(stream << mse::StatusCode::invalid, std::invalid_argument);
            }
        }
    }
}
