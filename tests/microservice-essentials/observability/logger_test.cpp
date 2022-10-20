#include <catch2/catch_test_macros.hpp>
#include <microservice-essentials/observability/logger.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <memory>

namespace
{
class TestLogger : public mse::Logger
{        
    public:
        TestLogger(mse::LogLevel min_log_level = mse::LogLevel::info, bool do_registration = false)
            : Logger(min_log_level)
            , _autoLogProviderRegistration(do_registration ? std::make_unique<mse::LogProvider::AutoRegistration>(*this) : nullptr)
        {
        }

        virtual void write(const mse::Context&, mse::LogLevel level, std::string_view message) override
        {
            _last_message = message;
            _last_level = level;
        }

        void clear()
        {
            _last_message = "";
            _last_level = mse::LogLevel::invalid;
        }

        std::string _last_message;
        mse::LogLevel _last_level = mse::LogLevel::invalid;
        std::unique_ptr<mse::LogProvider::AutoRegistration> _autoLogProviderRegistration;
};

}

SCENARIO( "Logger", "[observability][logging]" )
{
    GIVEN("a test logger with debug as min log level")
    {
        TestLogger logger(mse::LogLevel::debug);
        WHEN("a message with default log level is written")
        {
            logger.Write("my message");
            THEN("the last message is set correctly")
            {
                REQUIRE(logger._last_message == "my message");            
            }
            AND_THEN("the message has been written with info level")
            {
                REQUIRE(logger._last_level == mse::LogLevel::info);
            }
        }
        WHEN("a message with debug level is written")
        {
            logger.Write(mse::LogLevel::debug, "my debug message");
            THEN("the last message is set correctly")
            {
                REQUIRE(logger._last_message == "my debug message");
            }
            AND_THEN("the last log level is set correctly")
            {
                REQUIRE(logger._last_level == mse::LogLevel::debug);
            }
        }
        logger.clear();
        WHEN("a message with trace level is written")
        {        
            logger.Write(mse::LogLevel::trace, "my trace message");
            THEN("the last message has not been modified")
            {
                REQUIRE(logger._last_message == "");
            }
            AND_THEN("the last log level has not been modified")
            {
                REQUIRE(logger._last_level == mse::LogLevel::invalid);
            }
        }

    }
}

SCENARIO( "LogProvider", "[observability][logging]" )
{
    GIVEN("a log provider with no logging instance")
    {
        mse::LogProvider::GetInstance().SetLogger(nullptr);
        WHEN("a message is written to the global logger")
        {
            THEN("nothing happens")
            {                
                REQUIRE_NOTHROW(mse::LogProvider::GetLogger().Write("test"));
            }
        }
    }

    GIVEN("a logging instance")
    {
        mse::Logger* my_logger = nullptr;
        {
            TestLogger logger(mse::LogLevel::info, true);
            my_logger = &logger;

            WHEN("the global logger instance is obtained")
            {
                mse::Logger* global_logger = &mse::LogProvider::GetLogger();            
                THEN("it points to the instance")
                {                    
                    REQUIRE(global_logger == my_logger);
                }
            }
            WHEN("a message is written to the global logger")
            {
                mse::LogProvider::GetLogger().Write("message 1");
                THEN("the message is written to that instance")
                {
                    REQUIRE(logger._last_message == "message 1");
                }            
            }

            AND_GIVEN("a second logger instance")
            {
                TestLogger logger2(mse::LogLevel::info, true);
                WHEN("a message is written to the global logger")
                {
                    mse::LogProvider::GetLogger().Write("message 2");
                    THEN("the message is written to the second instance only")
                    {
                        REQUIRE(logger._last_message == "");
                        REQUIRE(logger2._last_message == "message 2");                        
                    }
                }
            }
        }
        WHEN("the instance is deleted")
        {
            THEN("the global logger does not point to that instance anymore")
            {
                REQUIRE(&mse::LogProvider::GetLogger() != my_logger);
            }
            AND_WHEN("a message is written to the global logger")
            {
                THEN("nothing happens")
                {
                    mse::LogProvider::GetLogger().Write("message 3");
                }
            }
        }
    }
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
    GIVEN("a console logger and cout and cerr redirected to buffer")
    {        
        std::ostringstream strCout, strCerr;
        std::streambuf* oldCoutStreamBuf = std::cout.rdbuf( strCout.rdbuf() );
        std::streambuf* oldCerrStreamBuf = std::cerr.rdbuf( strCerr.rdbuf() );
    

        mse::ConsoleLogger logger(mse::LogLevel::trace, mse::LogLevel::warn);

        WHEN("a message is written with info level")
        {
            logger.Write(mse::LogLevel::info, "info");
            THEN("the message is written to standard output")
            {
                REQUIRE(strCout.str() == "info\n");
            }
        }

        WHEN("a message is written with warn level")
        {
            logger.Write(mse::LogLevel::warn, "warn");
            THEN("the message is written to standard error")
            {
                REQUIRE(strCerr.str() == "warn\n");
            }
        }

        std::cout.rdbuf(oldCoutStreamBuf);
        std::cerr.rdbuf(oldCerrStreamBuf);
    }

}

SCENARIO( "DiscardLogger", "[observability][logging]" )
{
    GIVEN("a discard logger")
    {
        mse::DiscardLogger logger;

        WHEN("a message is written")
        {
            THEN("nothing happens")
            {
                REQUIRE_NOTHROW(logger.Write("some message"));
            }
        }
    }
}

SCENARIO("StructuredLogger", "[observability][logging]")
{    
    GIVEN("a structured logger with a TestLogger backend")
    {
        TestLogger test_logger;
        mse::StructuredLogger structured_logger(test_logger, {"message"});
        WHEN("a log message is written")
        {
            structured_logger.Write("test");
            THEN("the message is written in json format")
            {
                REQUIRE(test_logger._last_message == "{\"message\":\"test\"}");
            }
        }
    }

    GIVEN("some context with metadata")
    {
        mse::Context context({{"a", "x"}, {"b", "y"}, {"c", "z"}});
        WHEN("The context is converted to json")
        {
            std::string json_string = mse::StructuredLogger::to_json(context, nullptr);
            THEN("it can be parsed")
            {
                nlohmann::json data = nlohmann::json::parse(json_string);
                
                AND_THEN("the metadata is present in the json")
                {
                    REQUIRE(data["a"].get<std::string>() == "x");
                    REQUIRE(data["b"].get<std::string>() == "y");
                    REQUIRE(data["c"].get<std::string>() == "z");
                }
            }
        }
    }
    
    GIVEN("some context with metadata including special characters in the values")
    {
        mse::Context context({{"a", "xtest\t\t\""}, {"b", "test\nytest\r"}, {"c", "\\z\ftestz\b"}, {"d" , ""}});
        WHEN("The context is converted to json")
        {
            std::string json_string = mse::StructuredLogger::to_json(context, nullptr);
            THEN("it can be parsed")
            {
                nlohmann::json data = nlohmann::json::parse(json_string);
                for(const auto& element : data.items())
                {
                    AND_THEN(std::string("the value of key ") + element.key() + " didn't change")
                    {
                        REQUIRE(element.value().get<std::string>() == context.GetMetadata().find(element.key())->second);
                    }                    
                }
            }
        }
    }
    GIVEN("some context with metadata including special characters in the keys")
    {
        mse::Context context({{"xtest\t\t\"", "a"}, {"test\nytest\r", "b"}, {"\\z\ftestz\b", "c"}});
        WHEN("The context is converted to json")
        {
            std::string json_string = mse::StructuredLogger::to_json(context, nullptr);
            THEN("it can be parsed")
            {
                nlohmann::json data = nlohmann::json::parse(json_string);
                AND_THEN("all keys do exist")
                {
                    REQUIRE(data.contains("xtest\t\t\""));
                    REQUIRE(data.contains("test\nytest\r"));
                    REQUIRE(data.contains("\\z\ftestz\b"));
                }
            }
        }
    }

    GIVEN("some context with metadata including multiple values for a single key")
    {
        mse::Context context({{"a", "value1"}, {"a", "value2"}});
        WHEN("The context is converted to json")
        {
            std::string json_string = mse::StructuredLogger::to_json(context, nullptr);
            std::cout << json_string << std::endl;
            THEN("it can be parsed")
            {
                nlohmann::json data = nlohmann::json::parse(json_string);
                AND_THEN("one of the values can be retrieved")
                {
                    REQUIRE(((data.at("a").get<std::string>() == "value1") || (data.at("a").get<std::string>() == "value2")));
                }
            }
        }
    }

}
