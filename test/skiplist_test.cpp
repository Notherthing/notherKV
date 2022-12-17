#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
#include "element/skiplist.hpp"

unsigned int Factorial( unsigned int number ) {
  return number > 1 ? Factorial(number-1)*number : 1;
}
TEST_CASE( "Factorials are computed", "[factorial]" ) {
    REQUIRE( Factorial(0) == 1 );
    REQUIRE( Factorial(10) == 3628800 );
}
