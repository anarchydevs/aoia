
//  (C) Copyright Daniel James 2005.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "./config.hpp"

#ifdef TEST_STD_INCLUDES
#  include <functional>
#else
#  include <boost/functional/hash/hash.hpp>
#endif

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include <boost/limits.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/type_traits/is_base_and_derived.hpp>

#include "./compile_time.hpp"

BOOST_AUTO_UNIT_TEST(pointer_tests)
{
    compile_time_tests((int**) 0);
    compile_time_tests((void**) 0);

    HASH_NAMESPACE::hash<int*> x1;
    HASH_NAMESPACE::hash<int*> x2;

    int int1;
    int int2;

    BOOST_CHECK(x1(0) == x2(0));
    BOOST_CHECK(x1(&int1) == x2(&int1));
    BOOST_CHECK(x1(&int2) == x2(&int2));
#if defined(TEST_EXTENSIONS)
    BOOST_CHECK(x1(&int1) == HASH_NAMESPACE::hash_value(&int1));
    BOOST_CHECK(x1(&int2) == HASH_NAMESPACE::hash_value(&int2));

    // This isn't specified in Peter's proposal:
    BOOST_CHECK(x1(0) == 0);
#endif
}
