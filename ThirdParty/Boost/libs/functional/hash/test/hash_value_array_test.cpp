
//  (C) Copyright Daniel James 2005.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// On some compilers hash_value isn't available for arrays, so I test it
// separately from the main array tests.

#include "./config.hpp"

#ifdef TEST_EXTENSIONS
#  ifdef TEST_STD_INCLUDES
#    include <functional>
#  else
#    include <boost/functional/hash/hash.hpp>
#  endif
#endif

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#ifdef TEST_EXTENSIONS

BOOST_AUTO_UNIT_TEST(array_int_test)
{
    const int array1[25] = {
        26, -43, 32, 65, 45,
        12, 67, 32, 12, 23,
        0, 0, 0, 0, 0,
        8, -12, 23, 65, 45,
        -1, 93, -54, 987, 3
    };
    HASH_NAMESPACE::hash<int[25]> hasher1;

    int array2[1] = {3};
    HASH_NAMESPACE::hash<int[1]> hasher2;

    int array3[2] = {2, 3};
    HASH_NAMESPACE::hash<int[2]> hasher3;

    BOOST_CHECK(hasher1(array1) == HASH_NAMESPACE::hash_value(array1));
    BOOST_CHECK(hasher2(array2) == HASH_NAMESPACE::hash_value(array2));
    BOOST_CHECK(hasher3(array3) == HASH_NAMESPACE::hash_value(array3));
}

BOOST_AUTO_UNIT_TEST(two_dimensional_array_test)
{
    int array[3][2] = {{-5, 6}, {7, -3}, {26, 1}};
    HASH_NAMESPACE::hash<int[3][2]> hasher;

    BOOST_CHECK(hasher(array) == HASH_NAMESPACE::hash_value(array));
}

#endif
