// Boost.Range library
//
//  Copyright Thorsten Ottosen 2003-2004. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//


#include <boost/detail/workaround.hpp>

#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x564))
#  pragma warn -8091 // supress warning in Boost.Test
#  pragma warn -8057 // unused argument argc/argv in Boost.Test
#endif

#include <boost/range.hpp>
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <vector>

//
// Generic range algorithm
//
template< class Rng >
typename boost::range_result_iterator<Rng>::type foo_algo( Rng& r )
{
        //
        // This will only compile for Rng = UDT if the qualified calls
        // find boost_range_XXX via ADL.
        //
        return boost::size(r) == 0u ? boost::begin(r) : boost::end(r);
}

namespace Foo
{
        //
        // Our sample UDT
        //
        struct X
        {
                typedef std::vector<int>       data_t;
                typedef data_t::iterator       iterator;
                typedef data_t::const_iterator const_iterator;
                typedef data_t::size_type      size_type;

                data_t vec;

                void push_back( int i )
                { vec.push_back(i); }
        };

        //
        // The required functions. No type-traits need
        // to be defined because X defines the proper set of
        // nested types.
        //
        inline X::iterator boost_range_begin( X& x )
        {
                return x.vec.begin();
        }


        inline X::const_iterator boost_range_begin( const X& x )
        {
                return x.vec.begin();
        }


    inline X::iterator boost_range_end( X& x )
        {
                return x.vec.end();
        }

        inline X::const_iterator boost_range_end( const X& x )
        {
                return x.vec.end();
        }

        inline X::size_type boost_range_size( const X& x )
        {
                return x.vec.size();
        }
}

void check_extension()
{
        Foo::X x;
        x.push_back(3);
        const Foo::X x2;

        foo_algo( x );
        foo_algo( x2 );
}

using boost::unit_test::test_suite;

test_suite* init_unit_test_suite( int argc, char* argv[] )
{
    test_suite* test = BOOST_TEST_SUITE( "Range Test Suite" );

    test->add( BOOST_TEST_CASE( &check_extension ) );

    return test;
}





