#include <catch2/catch_test_macros.hpp>
#include <microservice-essentials/context.h>
#include <algorithm>
#include <iomanip>
#include <vector>

SCENARIO("Context Metadata", "[context]")
{
    GIVEN("Empty Context")
    {
        mse::Context context;
        THEN("random metadata is not available")
        {
            REQUIRE(context.GetMetadata().size() == 0);
            REQUIRE(context.GetAllMetadata().size() == 0);
            REQUIRE(context.Contains("test") == false);
        }
        AND_THEN("getting random metadata results in out of range exception")
        {
            REQUIRE_THROWS_AS(context.At("test"), std::out_of_range);
        }

        WHEN("some data is inserted")
        {
            context.Insert("test", "some_value");
            context.Insert({{"a", "x"}, {"b", "y"}});
            THEN("the data can be retrieved")
            {
                REQUIRE(context.Contains("test") == true);
                REQUIRE(context.At("test") == "some_value");
                REQUIRE(context.Contains("a") == true);
                REQUIRE(context.At("a") == "x");
                REQUIRE(context.Contains("b") == true);
                REQUIRE(context.At("b") == "y");
            }
            AND_THEN("the metadata multimap contains that data")
            {
                REQUIRE(context.GetMetadata().find("test") != context.GetMetadata().cend());
                auto allMetaData = context.GetAllMetadata();
                REQUIRE(allMetaData.find("test") != allMetaData.cend());
                REQUIRE(allMetaData.find("a") != allMetaData.cend());
                REQUIRE(allMetaData.find("b") != allMetaData.cend());
            }

            AND_WHEN("context is cleared")
            {
                context.Clear();
                THEN("data is not available anymore")
                {
                    REQUIRE(context.Contains("test") == false);
                }
            }
        }
    }

    GIVEN("Context with metadata")
    {
        mse::Context context({ { "a" , "x"}, {"b" , "y"}, {"c" , "z"} });
        WHEN("a filtered subset of the data is retrieved")
        {
            mse::Context::MetadataVector filtered_metadata = context.GetFilteredMetadata({"a", "c"});
            THEN("that subset is available in the given order")
            {                
                REQUIRE(filtered_metadata.size() == 2);
                REQUIRE(filtered_metadata[0].first ==  "a");
                REQUIRE(filtered_metadata[0].second ==  "x");
                REQUIRE(filtered_metadata[1].first ==  "c");
                REQUIRE(filtered_metadata[1].second ==  "z");                
            }
            AND_THEN("others are not available")
            {
                auto cit = std::find_if(filtered_metadata.begin(), filtered_metadata.end(), [](const mse::Context::MetadataVector::value_type& key_value_pair){ return key_value_pair.first == "y"; });
                REQUIRE(cit == filtered_metadata.end());
            }
        }

        WHEN("a filtered subset of the data is retrieved in reverse order")
        {
            mse::Context::MetadataVector filtered_metadata = context.GetFilteredMetadata({"c", "a"});
            THEN("that subset is available in the reverse order")
            {                
                REQUIRE(filtered_metadata.size() == 2);
                REQUIRE(filtered_metadata[1].first ==  "a");
                REQUIRE(filtered_metadata[1].second ==  "x");
                REQUIRE(filtered_metadata[0].first ==  "c");
                REQUIRE(filtered_metadata[0].second ==  "z");
            }
        }
        
    }
}

SCENARIO("Context with Parent", "[context]")
{
    GIVEN("A context with metadata")
    {
        mse::Context context;
        context.Insert("test", "value");
        WHEN("Another context with this context as parent is created")
        {
            mse::Context another_context(&context);
            THEN("data can be accessed via both instances")
            {
                REQUIRE(context.Contains("test") == true);
                REQUIRE(another_context.Contains("test") == true);
                REQUIRE(context.At("test") == "value");
                REQUIRE(another_context.At("test") == "value");
            }
            AND_WHEN("more metadata is inserted into the 2nd context")
            {
                another_context.Insert("test2", "value2");
                THEN("another_context contains both metadatas")
                {
                    REQUIRE(another_context.GetAllMetadata().size() == 2);
                    REQUIRE(another_context.Contains("test") == true);
                    REQUIRE(another_context.Contains("test2") == true);
                    REQUIRE(another_context.At("test") == "value");
                    REQUIRE(another_context.At("test2") == "value2");
                }

                AND_WHEN("another_context is cleared")
                {
                    another_context.Clear();
                    THEN("it only contains data from the parent")
                    {
                        REQUIRE(another_context.Contains("test") == true);
                        REQUIRE(another_context.Contains("test2") == false);
                    }
                }
            }
            
        }
    }
}

SCENARIO("Context Initialization", "[context]")
{
    WHEN("a context is constructed from an initializer list")
    {
        mse::Context context({{"a","x"}, {"b","y"}, {"c", "z"}});
        THEN("the context contains all the metadata")
        {
            REQUIRE(context.GetMetadata().size() == 3);
            REQUIRE(context.Contains("a"));
            REQUIRE(context.At("a") == "x");
            REQUIRE(context.Contains("b"));
            REQUIRE(context.At("b") == "y");
            REQUIRE(context.Contains("c"));
            REQUIRE(context.At("c") == "z");

        }
    }

    WHEN("a local context is constructed")
    {        
        mse::Context context = MSE_LOCAL_CONTEXT;
        const int context_line = __LINE__ -1;

        THEN("the metadata holds the filename")
        {
            REQUIRE(context.At("file").find("context_test.cpp") != std::string::npos);
        }
        AND_THEN("the metadata holds the function")
        {
            REQUIRE(context.At("function").find("CATCH2_INTERNAL_TEST") != std::string::npos);
        }
        AND_THEN("the metadata holds the line")
        {
            REQUIRE(context.At("line") == std::to_string(context_line));
        }
        AND_THEN("the metadata holds the timestamp in UTZ format")
        {
            REQUIRE(context.Contains("timestamp"));
            std::string timestamp = context.At("timestamp");
            std::stringstream timestampStream(timestamp);
            std::tm tmb = {};            
            timestampStream >> std::get_time(&tmb, "%Y-%m-%dT%H:%M:%SZ");
            REQUIRE(!timestampStream.fail());
            std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(timegm (&tmb));
            using namespace std::chrono_literals;
            REQUIRE((std::chrono::system_clock::now() - tp) < 2s);
        }
    }
}
