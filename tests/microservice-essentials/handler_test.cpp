#include <catch2/catch_test_macros.hpp>
#include <microservice-essentials/context.h>


TEST_CASE( "Handler", "[handler]" ) 
{
    mse::Context ctx;
    REQUIRE( 1 == 1 );
}