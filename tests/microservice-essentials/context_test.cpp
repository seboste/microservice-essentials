#include <catch2/catch_test_macros.hpp>
#include <microservice-essentials/context.h>
#include <algorithm>
#include <future>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#define timegm _mkgmtime
#endif

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
        AND_THEN("getting random metadata with default results in the default")
        {
            REQUIRE(context.AtOr("test", "default") == "default");
        }

        WHEN("some data is inserted")
        {
            context.Insert("test", "some_value");
            context.Insert({{"a", "x"}, {"b", "y"}});
            THEN("the data can be retrieved")
            {
                REQUIRE(context.Contains("test") == true);
                REQUIRE(context.At("test") == "some_value");
                REQUIRE(context.AtOr("test","default") == "some_value");
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

            AND_WHEN("some meta data is erased")
            {
                size_t erase_count = context.Erase("test");
                THEN("1 entry has been erased")
                {
                    REQUIRE(erase_count == 1);
                }
                AND_THEN("the erased data is not there anymore")
                {
                    REQUIRE(context.Contains("test") == false);
                }
                AND_THEN("all other metadata is still there")
                {
                    REQUIRE(context.Contains("a") == true);
                    REQUIRE(context.Contains("b") == true);
                }
            }

            AND_WHEN("the same meta data is inserted again")
            {
                context.Insert("test", "some_other_value");
                THEN("one of the values is returned")
                {
                    REQUIRE((context.At("test") == "some_other_value" || context.At("test") == "some_value"));
                }

                AND_WHEN("that duplicated data is erased")
                {
                    size_t erase_count = context.Erase("test");
                    THEN("both entries are not there anymore")
                    {
                        REQUIRE(erase_count == 2);
                        REQUIRE(context.Contains("test") == false);
                    }                    
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
                REQUIRE(context.AtOr("test", "default") == "value");
                REQUIRE(another_context.At("test") == "value");
                REQUIRE(another_context.AtOr("test", "default") == "value");
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
                    REQUIRE(another_context.AtOr("test", "default") == "value");
                    REQUIRE(another_context.AtOr("test2", "default") == "value2");
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

                AND_WHEN("the metadata from another_context is erased")
                {
                    size_t erased_count = another_context.Erase("test2");
                    THEN("1 entry has been erased")
                    {
                        REQUIRE(erased_count == 1);
                    }
                    THEN("it only contains data from the parent")
                    {
                        REQUIRE(another_context.Contains("test") == true);
                        REQUIRE(another_context.Contains("test2") == false);
                    }
                }

                AND_WHEN("the parent metadata is attempted to be erased from anoterh_context")
                {
                    size_t erased_count = another_context.Erase("test");
                    THEN("nothing has been erased")
                    {
                        REQUIRE(erased_count == 0);
                    }
                    THEN("both entries are still present")
                    {
                        REQUIRE(another_context.Contains("test") == true);
                        REQUIRE(another_context.Contains("test2") == true);
                    }
                }

                AND_WHEN("the parent metadata is erased from the parent context")
                {
                    size_t erased_count = context.Erase("test");
                    THEN("1 entry has been erased")
                    {
                        REQUIRE(erased_count == 1);
                    }
                    THEN("it only contains data from another_context")
                    {
                        REQUIRE(another_context.Contains("test") == false);
                        REQUIRE(another_context.Contains("test2") == true);
                    }
                }

                AND_WHEN("the same meta data is inserted")
                {
                    another_context.Insert("test", "another_value");
                    THEN("the current context takes precedence")
                    {
                        REQUIRE(context.At("test") == "value");
                        REQUIRE(another_context.At("test") == "another_value");
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


SCENARIO("Context Parent", "[context]")
{
    GIVEN("a global context with some metadata")
    {
        if(!mse::Context::GetGlobalContext().Contains("global"))
        {
            mse::Context::GetGlobalContext().Insert({{"global", "foo"}});
        }
        AND_GIVEN("a thread local context with some metadata")
        {
            if(!mse::Context::GetThreadLocalContext().Contains("thread_local"))
            {
                mse::Context::GetThreadLocalContext().Insert({{"thread_local", "bar"}});
            }            
            THEN("the thread local context contains global and thread local metadata")
            {
                REQUIRE(mse::Context::GetThreadLocalContext().Contains("global"));
                REQUIRE(mse::Context::GetThreadLocalContext().Contains("thread_local"));
            }
            WHEN("a context without a parent is created")
            {
                mse::Context context({{"local","some_value"}}, nullptr);
                THEN("that context contains all metadata")
                {
                    REQUIRE(context.Contains("global"));
                    REQUIRE(context.Contains("thread_local"));
                    REQUIRE(context.Contains("local"));
                }

                AND_WHEN("the thread local context is cleared")
                {
                    mse::Context::GetThreadLocalContext().Clear();
                    THEN("the context contains all metadata but the thread local one")
                    {
                        REQUIRE(context.Contains("global"));
                        REQUIRE(!context.Contains("thread_local"));
                        REQUIRE(context.Contains("local"));
                    }
                }
            }
            WHEN("a thread local context in another thread is accessed")
            {
                auto future = std::async([]()
                {
                    const mse::Context& thread_local_context = mse::Context::GetThreadLocalContext(); 
                    THEN("the thread local context contains the global but not the thread local metadata from the other thread")
                    {
                        REQUIRE(thread_local_context.Contains("global"));
                        REQUIRE(!thread_local_context.Contains("thread_local"));
                    }                    
                });                
                future.get();
                REQUIRE(mse::Context::GetThreadLocalContext().Contains("thread_local"));
            }
            WHEN("a thread_local context of another thread is initialized with this thread's thread_local context")            
            {
                const mse::Context& thread_local_context = mse::Context::GetThreadLocalContext();
                auto future = std::async([thread_local_context]()
                {
                    mse::Context::GetThreadLocalContext() = mse::Context(thread_local_context);
                    mse::Context::GetThreadLocalContext().Insert({{"thread_local_child", "bar"}});
                    THEN("the thread local context contains the global but not the thread local metadata from the other thread")
                    {
                        REQUIRE(mse::Context::GetThreadLocalContext().Contains("global"));
                        REQUIRE(mse::Context::GetThreadLocalContext().Contains("thread_local"));
                        REQUIRE(mse::Context::GetThreadLocalContext().Contains("thread_local_child"));
                    }                    
                });                           
                future.get();
                AND_THEN("the parent thread does not contain the child threads metadata")
                {
                    REQUIRE(!mse::Context::GetThreadLocalContext().Contains("thread_local_child"));
                }
            }
            WHEN("an empty context is copied to the global context")
            {
                REQUIRE(mse::Context::GetGlobalContext().Contains("global"));
                mse::Context context;
                mse::Context::GetGlobalContext() = context;
                THEN("the global context doesn't contain the metadata anymore")
                {
                    REQUIRE(!mse::Context::GetGlobalContext().Contains("global"));
                }
            }
            WHEN("an empty context is moved to the global context")
            {
                REQUIRE(mse::Context::GetGlobalContext().Contains("global"));
                mse::Context context;
                mse::Context::GetGlobalContext() = std::move(context);
                THEN("the global context doesn't contain the metadata anymore")
                {
                    REQUIRE(!mse::Context::GetGlobalContext().Contains("global"));
                }
            }
            WHEN("an empty context is copied to the thread_local context")
            {
                REQUIRE(mse::Context::GetGlobalContext().Contains("global"));
                REQUIRE(mse::Context::GetThreadLocalContext().Contains("thread_local"));
                mse::Context context;                
                mse::Context::GetThreadLocalContext() = context;
                THEN("the thread_local context contains the global metadata")
                {
                    REQUIRE(mse::Context::GetThreadLocalContext().Contains("global"));
                }
                AND_THEN("the thread_local context doesn't contain the thread_local metadata anymore")
                {
                    REQUIRE(!mse::Context::GetThreadLocalContext().Contains("thread_local"));
                }
            }
        }
    }
    GIVEN("a context with a parent context")
    {
        if(!mse::Context::GetThreadLocalContext().Contains("thread_local"))
        {
            mse::Context::GetThreadLocalContext().Insert({{"thread_local", "tl"}});
        }

        mse::Context parent({{"parent", "foo"}});
        mse::Context child({{"child", "bar" }}, &parent);
        WHEN("the child is copied to the parent")
        {
            parent = child;
            THEN("the parent contains the thread local meta data")
            {
                REQUIRE(parent.Contains("thread_local"));
            }
            THEN("the parent contains the child meta data")
            {
                REQUIRE(parent.Contains("child"));
            }
            THEN("the parent does NOT contain the parent meta data")
            {
                REQUIRE(!parent.Contains("parent"));
            }
        }

        WHEN("the parent is copied to the child")
        {
            child = parent;
            THEN("the child contains the thread local meta data")
            {
                REQUIRE(child.Contains("thread_local"));
            }
            THEN("the child contains the parent meta data")
            {
                REQUIRE(child.Contains("parent"));
            }
            THEN("the child does NOT contain the child meta data")
            {
                REQUIRE(!child.Contains("child"));
            }
        }

        AND_GIVEN("a grand child context")
        {
            mse::Context grand_child({{"grand_child", "bar" }}, &child);

            WHEN("the grand child is copied to the grand parent")
            {
                THEN("a logic error is thrown")
                {
                    REQUIRE_THROWS_AS(parent = grand_child, std::logic_error);
                }                
            }

            WHEN("the grand parent is copied to the grand child")
            {                
                THEN("no exception is thrown")
                {
                    REQUIRE_NOTHROW(grand_child = parent);
                }
            }
        }
    }
}