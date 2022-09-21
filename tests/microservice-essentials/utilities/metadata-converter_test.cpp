#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <microservice-essentials/utilities/metadata-converter.h>
#include <string_view>


SCENARIO( "Metadata converter", "[metadata][context]" )
{
    GIVEN("some multimap similar to the one returned by grpc::ServerContext::client_metadata()")
    {
        typedef std::multimap<std::string_view, std::string_view> ExternalMetadata;

        const ExternalMetadata external_metadata = { { "a", "x"}, {"b", "y"} };
        WHEN("it is converted to context metadata")
        {
            const mse::Context::MetaData metadata = mse::ToContextMetadata(external_metadata);
            THEN("the result contains the same elements as the original")
            {
                REQUIRE(external_metadata.size() == metadata.size());                
                auto citA = metadata.find("a");
                REQUIRE(citA != metadata.end());
                if(citA != metadata.end())
                {
                    REQUIRE(citA->second == "x" );
                }
                auto citB = metadata.find("b");
                REQUIRE(citB != metadata.end());
                if(citB != metadata.end())
                {
                    REQUIRE(citB->second == "y" );
                }
            }
        }
    }
}
