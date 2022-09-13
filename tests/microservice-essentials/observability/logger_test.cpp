#include <catch2/catch_test_macros.hpp>
#include <microservice-essentials/observability/logger.h>
#include <exception>

SCENARIO( "Logger", "[observability][logging]" )
{
}

SCENARIO( "LogProvider", "[observability][logging]" )
{
}

SCENARIO( "LogLevel", "[observability][logging]" )
{
    const std::vector<std::string> log_levels = { "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "CRITICAL" }; //increasing
    REQUIRE(log_levels.size() == (static_cast<int>(mse::LogLevel::highest) - static_cast<int>(mse::LogLevel::lowest) + 1) );
    
    int level_number = 0;
    for(auto level_string : log_levels)
    {
        GIVEN(std::string("the log level ") + level_string)
        {
            std::stringstream level_stream(level_string);
            WHEN("it is converted to a log_level")
            {
                mse::LogLevel level = mse::LogLevel::invalid;
                mse::from_string(level_string, level);
                THEN("the level is not invalid")
                {
                    REQUIRE(level != mse::LogLevel::invalid);
                }
                AND_THEN("the level number is increasing")
                {
                    REQUIRE(static_cast<int>(level) == level_number);
                }
                AND_WHEN("the level is converted to a string")
                {
                    std::string converted_level_string = mse::to_string(level);
                    THEN("the string is identical to the original level string")
                    {
                        REQUIRE(converted_level_string == level_string);
                    }
                }
                AND_WHEN("the >> operator is used to do the conversion")
                {
                    mse::LogLevel level_from_stream = mse::LogLevel::invalid;
                    bool result = (level_stream >> level_from_stream);
                    THEN("the conversion succeeds")
                    {
                        REQUIRE(result == true);
                    }
                    AND_THEN("it is identical to the normal conversion")
                    {
                        REQUIRE(level_from_stream == level);
                    }
                }
                AND_WHEN("the << operator is used to write the level into a stream")
                {
                    std::stringstream stream;
                    std::ostream& result = (stream << level);
                    THEN("the return value refers to the input stream")
                    {
                        REQUIRE(&result == static_cast<std::ostream*>(&stream));
                    }
                    AND_THEN("the stream consists of the original level string")
                    {
                        REQUIRE(stream.str() == level_string);
                    }
                }
            }            
        }        
        level_number++;
    }
    
    GIVEN("an invalid level string")
    {
        const std::string invalid_level_string = "BLA";
        std::stringstream invalid_level_stream(invalid_level_string);
        WHEN("it is converted to a log_level")
        {
            mse::LogLevel level = mse::LogLevel::err;
            mse::from_string(invalid_level_string, level);
            THEN("the level is invalid")
            {
                REQUIRE(level == mse::LogLevel::invalid);
            }        
        }
        WHEN("the >> operator is used to do the conversion")
        {
            mse::LogLevel level_from_stream = mse::LogLevel::err;
            bool result = (invalid_level_stream >> level_from_stream);
            THEN("the conversion fails")
            {
                REQUIRE(result == false);
            }
            AND_THEN("the converted level is invalid")
            {
                REQUIRE(level_from_stream == mse::LogLevel::invalid);
            }
        }
    }
    GIVEN("an invalid level")
    {
        WHEN("it is converted to a string")
        {
            THEN("an invalid argument exception is thrown")
            {
                REQUIRE_THROWS_AS(mse::to_string(mse::LogLevel::invalid), std::invalid_argument);
            }
        }
        WHEN("it is put into a stream using the << operator")
        {
            std::stringstream stream;
            THEN("an invalid argument exception is thrown")
            {
                REQUIRE_THROWS_AS(stream << mse::LogLevel::invalid, std::invalid_argument);
            }
        }
    }
}

SCENARIO( "ConsoleLogger", "[observability][logging]" )
{
}

SCENARIO( "DiscardLogger", "[observability][logging]" )
{
}
   