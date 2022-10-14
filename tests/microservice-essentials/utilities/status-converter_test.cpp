#include <catch2/catch_test_macros.hpp>
#include <microservice-essentials/utilities/status-converter.h>
#include <string_view>

namespace
{
enum class MyStatusCode //just some random subset
{
    OK = 0,
    UNIMPLEMENTED = 12,
    DATA_LOSS = 15,

    INVALID_CODE = 100
};

class MyStatus
{
    public:        
        MyStatus(MyStatusCode code, const std::string& error_message)
            : _code(code), _msg(error_message) {}
        MyStatus() : MyStatus(MyStatusCode::OK, "") {}

        MyStatusCode error_code() const { return _code; }
        std::string error_message() const { return _msg; }

        MyStatusCode _code;
        std::string _msg;
};
}

SCENARIO( "GRPC Status converter", "[status][grpc]" )
{
    GIVEN("a grpc status like class")
    {
        WHEN("a status is converted from that class")
        {
            mse::Status status = mse::FromGrpcStatus(MyStatus(MyStatusCode::UNIMPLEMENTED, "unimplemented"));
            THEN("the corresponding code is set")
            {
                REQUIRE(status.code == mse::StatusCode::unimplemented);                
            }
            THEN("the message is set correctly")
            {
                REQUIRE(status.details == "unimplemented");
            }
        }

        WHEN("a status is converted from that class with an invalid code")
        {
            THEN("an std::out_of_range exception is thrown")
            {
                REQUIRE_THROWS_AS(mse::FromGrpcStatus(MyStatus(MyStatusCode::INVALID_CODE, "bla")), std::out_of_range);
            }
        }

        WHEN("a status is converted to that class")
        {
            MyStatus my_status = mse::ToGrpcStatus<MyStatus>(mse::Status{ mse::StatusCode::data_loss, "data loss"});
            THEN("the corresponding code is set")
            {
                REQUIRE(my_status._code == MyStatusCode::DATA_LOSS);
            }
            THEN("the message is set correctly")
            {
                REQUIRE(my_status._msg == "data loss");
            }
        }
    }
}

SCENARIO( "HTTP Status Code converter", "[status][http]" )
{
    WHEN("a 200 http status code (success) is converted to a StatusCode")
    {
        mse::StatusCode status_code = mse::FromHttpStatusCode(200);
        THEN("the status code is OK")
        {
            REQUIRE(status_code == mse::StatusCode::ok);
        }
    }

    WHEN("a 404 http status code (not found) is converted to a StatusCode")
    {
        mse::StatusCode status_code = mse::FromHttpStatusCode(404);
        THEN("the status code is not found")
        {
            REQUIRE(status_code == mse::StatusCode::not_found);
        }
    }

    WHEN("some random 4xx http status code is converted to a StatusCode")
    {
        mse::StatusCode status_code = mse::FromHttpStatusCode(456);
        THEN("the status code indicates a client error with invalid argument")
        {
            REQUIRE(status_code == mse::StatusCode::invalid_argument);
        }
    }

    WHEN("some random 5xx http status code is converted to a StatusCode")
    {
        mse::StatusCode status_code = mse::FromHttpStatusCode(567);
        THEN("the status code indicates a server error with internal")
        {
            REQUIRE(status_code == mse::StatusCode::internal);
        }
    }

    WHEN("some random non existing http status code is converted to a StatusCode")
    {
        THEN("an std::out_of_range exception is thrown")
        {
            REQUIRE_THROWS_AS(mse::FromHttpStatusCode(789), std::out_of_range);
        }
    }

    GIVEN("a user defined error code lut")
    {
        mse::HttpStatusCodeToStatusCodeLookupTables lut = 
        {
            { { 8, mse::StatusCode::data_loss} },
            { { 867, mse::StatusCode::cancelled} }
        };

        WHEN("some specific existing code is converted")
        {
            mse::StatusCode status_code = mse::FromHttpStatusCode(867, lut);
            THEN("the specific lut's status code is returned ")
            {
                REQUIRE(status_code == mse::StatusCode::cancelled);
            }
        }

        WHEN("some range based existing code is converted")
        {
            mse::StatusCode status_code = mse::FromHttpStatusCode(868, lut);
            THEN("the general lut's status code is returned ")
            {
                REQUIRE(status_code == mse::StatusCode::data_loss);
            }
        }

        WHEN("some non existing code is converted")
        {
            THEN("an std::out_of_range exception is thrown")
            {
                REQUIRE_THROWS_AS(mse::FromHttpStatusCode(123, lut), std::out_of_range);
            }
        }
    }

    WHEN("a OK status code is converted to a http status code")
    {
        int http_status_code = mse::ToHttpStatusCode(mse::StatusCode::ok);
        THEN("the http status code is 200 (OK)")
        {
            REQUIRE(http_status_code == 200);
        }
    }

    WHEN("a permission denied code is converted to a http status code")
    {
        int http_status_code = mse::ToHttpStatusCode(mse::StatusCode::permission_denied);
        THEN("the http status code is 403 (Forbidden)")
        {
            REQUIRE(http_status_code == 403);
        }
    }

    WHEN("a unavailable code is converted to a http status code")
    {
        int http_status_code = mse::ToHttpStatusCode(mse::StatusCode::unavailable);
        THEN("the http status code is 503 (Service Unavailable)")
        {
            REQUIRE(http_status_code == 503);
        }
    }

    WHEN("an invalid code is converted to a http status code")
    {
        THEN("an std::invalid_argument exception is thrown")
        {
            REQUIRE_THROWS_AS(mse::ToHttpStatusCode(mse::StatusCode::invalid), std::invalid_argument);
        }
    }

    GIVEN("a user defined error code lut")
    {
        mse::StatusCodeToHttpStatusCodeLookupTable lut = 
        {
            { mse::StatusCode::data_loss , 4711 }            
        };

        WHEN("some specific existing code is converted")
        {
            int http_status_code = mse::ToHttpStatusCode(mse::StatusCode::data_loss, lut);
            THEN("the lut's http code is returned ")
            {
                REQUIRE(http_status_code == 4711);
            }
        }

        WHEN("some non existing code is converted")
        {
            THEN("an std::out_of_range exception is thrown")
            {
                REQUIRE_THROWS_AS(mse::ToHttpStatusCode(mse::StatusCode::cancelled, lut), std::out_of_range);
            }
        }
    }
}
