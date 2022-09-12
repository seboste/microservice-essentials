#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <microservice-essentials/utilities/environment.h>

#include <iostream>

using namespace std::chrono_literals;

SCENARIO( "Environment variables", "[utilities][environment]" )
{
    GIVEN("environment variable exists")
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
        WHEN("float var exp is read")
        {
            std::optional<float> opt_val = mse::getenv_optional<float>("TEST_FLOAT_EXP_VAR");
            THEN("value is available") { REQUIRE(opt_val.has_value()); }
            AND_THEN("value is correct") { REQUIRE(opt_val.value() == Catch::Approx(-1.25e-3f)); }
        }
        WHEN("double var is read")
        {
            std::optional<double> opt_val = mse::getenv_optional<double>("TEST_DOUBLE_VAR");
            THEN("value is available") { REQUIRE(opt_val.has_value()); }
            AND_THEN("value is correct") { REQUIRE(opt_val.value() == Catch::Approx(-1.23456)); }
        }
        WHEN("bool var false is read")
        {
            std::optional<bool> opt_val = mse::getenv_optional<bool>("TEST_BOOL_VAR_FALSE");
            THEN("value is available") { REQUIRE(opt_val.has_value()); }
            AND_THEN("value is correct") { REQUIRE(opt_val.value() == false); }
        }
        WHEN("bool var true is read")
        {
            std::optional<bool> opt_val = mse::getenv_optional<bool>("TEST_BOOL_VAR_TRUE");
            THEN("value is available") { REQUIRE(opt_val.has_value()); }
            AND_THEN("value is correct") { REQUIRE(opt_val.value() == true); }
        }
    }

    GIVEN("string environment variable exists")
    {
        putenv(const_cast<char*>("TEST_STRING_VAR=string"));
        
        WHEN("string var is read")
        {
            THEN("no exception is thrown")
            {
                REQUIRE_NOTHROW(mse::getenv<std::string>("TEST_STRING_VAR"));
            }
            const std::string val = mse::getenv<std::string>("TEST_STRING_VAR");
            AND_THEN("value is correct")
            {                
                REQUIRE(val == "string");
            }
        }
        WHEN("string is read with const char* default")
        {
            const std::string val = mse::getenv_or("TEST_STRING_VAR", "default");
            THEN("value is correct")
            {
                REQUIRE(val == "string");
            }
        }
        WHEN("string is read with std::string default")
        {
            const std::string val = mse::getenv_or("TEST_STRING_VAR", std::string("default"));
            THEN("value is correct")
            {
                REQUIRE(val == "string");
            }
        }
        WHEN("string is errorneously read as float")
        {
            THEN("an invalid argument exception is thrown")
            {                
                REQUIRE_THROWS_AS(mse::getenv<float>("TEST_STRING_VAR"), std::invalid_argument);
            }            
        }
    }

    GIVEN("environment variable does not exist")
    {
        putenv(const_cast<char*>("TEST_NONEXISTING_VAR"));
        WHEN("that variable is read as optional")
        {
            std::optional<std::string> val_opt = mse::getenv_optional<std::string>("TEST_NONEXISTING_VAR");
            THEN("result doesn't have a value")
            {
                REQUIRE(val_opt.has_value() == false);
            }
        }
        WHEN("that variable is read as required")
        {
            THEN("an invalid argument exception is thrown")
            {
                REQUIRE_THROWS_AS(mse::getenv<std::string>("TEST_NONEXISTING_VAR"), std::invalid_argument);
            }
        }
        WHEN("that variable is read with string default")
        {
            std::string val = mse::getenv_or("TEST_NONEXISTING_VAR", "default");
            THEN("the value is the default")
            {
                REQUIRE(val == "default");
            }
        }
        WHEN("that variable is read with float default")
        {
            float val = mse::getenv_or("TEST_NONEXISTING_VAR", 2.3f);
            THEN("the value is the default")
            {
                REQUIRE(val == Catch::Approx(2.3f));
            }
        }
    }
}
