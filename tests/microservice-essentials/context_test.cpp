#include <catch2/catch_test_macros.hpp>
#include <microservice-essentials/context.h>

SCENARIO("Context Metadata")
{
    GIVEN("Empty Context")
    {
        mse::Context context;
        THEN("random metadata is not available")
        {
            REQUIRE(context.GetMetaData().size() == 0);
            REQUIRE(context.GetAllMetaData().size() == 0);
            REQUIRE(context.Contains("test") == false);
        }
        AND_THEN("getting random metadata results in out of range exception")
        {
            REQUIRE_THROWS_AS(context.At("test"), std::out_of_range);
        }

        WHEN("some data is inserted")
        {
            context.Insert("test", "some_value");
            THEN("the data can be retrieved")
            {
                REQUIRE(context.Contains("test") == true);
                REQUIRE(context.At("test") == "some_value");
            }
            AND_THEN("the metadata multimap contains that data")
            {
                REQUIRE(context.GetMetaData().find("test") != context.GetMetaData().cend());
                auto allMetaData = context.GetAllMetaData();
                REQUIRE(allMetaData.find("test") != allMetaData.cend());
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
}

SCENARIO("Parent Context")
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
                    REQUIRE(another_context.GetAllMetaData().size() == 2);
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
