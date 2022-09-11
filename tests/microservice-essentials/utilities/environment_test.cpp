#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <microservice-essentials/utilities/environment.h>

using namespace std::chrono_literals;

SCENARIO( "Environment variables", "[utilities][environment]" )
{
    GIVEN("string environment variable exists")
    {
        putenv(const_cast<char*>("TEST_STRING_VAR=string"));
        putenv(const_cast<char*>("TEST_FLOAT_VAR=1.0"));
        putenv(const_cast<char*>("TEST_FLOAT_EXP_VAR=-1.25e-3"));
        putenv(const_cast<char*>("TEST_INT_VAR=-5"));
        putenv(const_cast<char*>("TEST_DOUBLE_VAR=-1.23456"));
        putenv(const_cast<char*>("TEST_BOOL_VAR_FALSE=0"));
        putenv(const_cast<char*>("TEST_BOOL_VAR_TRUE=1"));

        WHEN("string var is read")
        {
            std::optional<std::string> opt_val = mse::getenv_optional<std::string>("TEST_STRING_VAR");
            THEN("value is available") { REQUIRE(opt_val.has_value()); }
            AND_THEN("value is correct") { REQUIRE(opt_val.value() == "string"); }
        }

        WHEN("float var is read")
        {
            std::optional<float> opt_val = mse::getenv_optional<float>("TEST_FLOAT_VAR");
            THEN("value is available") { REQUIRE(opt_val.has_value()); }
            AND_THEN("value is correct") { REQUIRE(opt_val.value() == Catch::Approx(1.0f)); }
        }
    }
}
