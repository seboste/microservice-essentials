#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <microservice-essentials/utilities/metadata-converter.h>
#include <string_view>


SCENARIO( "Metadata converter", "[metadata][context]" )
{
    typedef std::multimap<std::string_view, std::string_view> ExternalMetadata;
    GIVEN("some multimap similar to the one returned by grpc::ServerContext::client_metadata()")
    {
        const ExternalMetadata external_metadata = { { "a", "x"}, {"b", "y"} };
        WHEN("it is converted to context metadata")
        {
            const mse::Context::Metadata metadata = mse::ToContextMetadata(external_metadata);
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

    GIVEN("some container with data in MiXed CaSe")
    {
        const ExternalMetadata external_metadata = { { "AbCdE123!", "XyZ?"} };
        WHEN("it is converted to context metadata with to lower conversion ON")
        {
            const mse::Context::Metadata metadata = mse::ToContextMetadata(external_metadata, true);
            THEN("the key is converted to lower case")
            {                
                auto cit = metadata.begin();
                REQUIRE(cit != metadata.end());
                REQUIRE(cit->first == "abcde123!");

                AND_THEN("the value case remains identical")
                {
                    REQUIRE(cit->second == "XyZ?");
                }
            }
        }

        WHEN("it is converted to context metadata with to lower conversion OFF")
        {
            const mse::Context::Metadata metadata = mse::ToContextMetadata(external_metadata, false);
            THEN("the key case remains identical")
            {                
                auto cit = metadata.begin();
                REQUIRE(cit != metadata.end());
                REQUIRE(cit->first == "AbCdE123!");

                AND_THEN("the value case remains identical")
                {
                    REQUIRE(cit->second == "XyZ?");
                }
            }
        }
    }

    GIVEN("some context metadata")
    {
        const mse::Context::Metadata metadata = { { "a", "x"}, {"b", "y"} };
        WHEN("it is converted to some multimap similar to the one returned by grpc::ServerContext::client_metadata()")
        {
            const ExternalMetadata external_metadata = mse::FromContextMetadata<ExternalMetadata>(metadata);
            THEN("the result contains the same elements as the original")
            {
                REQUIRE(metadata.size() == external_metadata.size());                
                auto citA = external_metadata.find("a");
                REQUIRE(citA != external_metadata.end());
                if(citA != external_metadata.end())
                {
                    REQUIRE(citA->second.compare("x") == 0);
                }
                auto citB = external_metadata.find("b");
                REQUIRE(citB != external_metadata.end());
                if(citB != external_metadata.end())
                {
                    REQUIRE(citB->second.compare("y") == 0);
                }
            }
        }

        AND_GIVEN("some struct with an AddMetaData function")
        {
            struct MyClientContext //similar to grpc::ClientContext
            {
                void AddMetadata(const std::string& key, std::string_view value)
                {
                    _metadata.insert({key, std::string(value) });
                }                
                std::map<std::string, std::string> _metadata;
            };

            MyClientContext cc;
            WHEN("the context metadata is exported")
            {
                mse::ExportMetadata(metadata, &MyClientContext::AddMetadata, cc);
                THEN("the result contains the same elements as the original")
                {
                    REQUIRE(metadata.size() == cc._metadata.size());                
                    auto citA = cc._metadata.find("a");
                    REQUIRE(citA != cc._metadata.end());
                    if(citA != cc._metadata.end())
                    {
                        REQUIRE(citA->second.compare("x") == 0);
                    }
                    auto citB = cc._metadata.find("b");
                    REQUIRE(citB != cc._metadata.end());
                    if(citB != cc._metadata.end())
                    {
                        REQUIRE(citB->second.compare("y") == 0);
                    }
                }
            }
        }
    }
}
