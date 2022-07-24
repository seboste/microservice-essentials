#include <catch2/catch.hpp>
#include <microservice-essentials/context.h>


TEST_CASE( "Context", "[context]" ) 
{
    microservice_essentials::Context ctx;
    REQUIRE( 1 == 1 );
}