#include <catch2/catch_test_macros.hpp>
#include <microservice-essentials/utilities/url.h>

SCENARIO( "UrlStringConversion", "[url]" )
{
    const std::vector<std::string> input = {
        "https://john.doe@www.example.com:123/forum/questions/?tag=networking&order=newest#top",
        "https://bla.com",
        "www.bla.com/test",
        "user:pwd@www.bla.com#abc",
        "www.bla.com:123?x=y",
        "www.bla.com:123/?x=y",
        "www.bla.com:123/blub?x=y",
    };

    //scheme, userinfo, host, port, path, query, fragment
    const std::vector<mse::Url> expected_result = {
        mse::Url{"https", "john.doe", "www.example.com" , "123" , "forum/questions/", "tag=networking&order=newest" , "top" },
        mse::Url{"https", ""        , "bla.com"         , ""    , ""                , ""                            , ""    },
        mse::Url{""     , ""        , "www.bla.com"     , ""    , "test"            , ""                            , ""    },
        mse::Url{""     , "user:pwd", "www.bla.com"     , ""    , ""                , ""                            , "abc" },
        mse::Url{""     , ""        , "www.bla.com"     , "123" , ""                , "x=y"                         , ""    },
        mse::Url{""     , ""        , "www.bla.com"     , "123" , ""                , "x=y"                         , ""    },
        mse::Url{""     , ""        , "www.bla.com"     , "123" , "blub"            , "x=y"                         , ""    },
    };

    const std::vector<std::string> expected_to_string = {
        "https://john.doe@www.example.com:123/forum/questions/?tag=networking&order=newest#top",
        "https://bla.com/",
        "www.bla.com/test",
        "user:pwd@www.bla.com/#abc",
        "www.bla.com:123/?x=y",
        "www.bla.com:123/?x=y",
        "www.bla.com:123/blub?x=y",
    };

    const std::vector<std::string> expected_connection_string = {
        "https://john.doe@www.example.com:123",
        "https://bla.com",
        "www.bla.com",
        "user:pwd@www.bla.com",
        "www.bla.com:123",
        "www.bla.com:123",
        "www.bla.com:123",
    };

    const std::vector<std::string> expected_whole_path = {
        "/forum/questions/?tag=networking&order=newest#top",
        "/",
        "/test",
        "/#abc",
        "/?x=y",
        "/?x=y",
        "/blub?x=y",
    };

    const std::vector<std::optional<uint32_t>> expected_port = {
        123,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        123,
        123,
        123,
    };

    for(uint32_t i = 0; i < input.size(); ++i)
    {
        GIVEN(input[i])
        {
            WHEN("it is converted to url")
            {
                mse::Url result = mse::ToUrl(input[i]);
                THEN("url is as expected")
                {
                    REQUIRE(expected_result[i].scheme == result.scheme);
                    REQUIRE(expected_result[i].userinfo == result.userinfo);
                    REQUIRE(expected_result[i].host == result.host);
                    REQUIRE(expected_result[i].port == result.port);
                    REQUIRE(expected_result[i].path == result.path);
                    REQUIRE(expected_result[i].query == result.query);
                    REQUIRE(expected_result[i].fragment == result.fragment);
                }
            }

            WHEN("the url is converted to a string")
            {
                std::string result = std::to_string(expected_result[i]);
                THEN("the expected string is returned")
                {
                    REQUIRE(result == expected_to_string[i]);
                }
            }

            WHEN("the GetWholePath() is called")
            {
                std::string result = expected_result[i].GetWholePath();
                THEN("the expected whole path is returned")
                {
                    REQUIRE(result == expected_whole_path[i]);
                }
            }

            WHEN("the GetConnectionString() is called")
            {
                std::string result = expected_result[i].GetConnectionString();
                THEN("the expected connection string is returned")
                {
                    REQUIRE(result == expected_connection_string[i]);
                }
            }

            WHEN("the GetPort() is called")
            {
                std::optional<uint32_t> result = expected_result[i].GetPort();
                THEN("the expected port is returned")
                {
                    REQUIRE(result == expected_port[i]);
                }
            }
        }
    }

    GIVEN("an empty string")
    {
        WHEN("it is converted to a url")
        {
            mse::Url url;
            bool success = mse::ToUrl("", url);
            THEN("this must fail")
            {
                REQUIRE(success == false);
            }
        }
    }
}

