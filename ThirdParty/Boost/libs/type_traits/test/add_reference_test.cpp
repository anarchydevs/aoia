
//  (C) Copyright John Maddock 2000. 
//  Use, modification and distribution are subject to the 
//  Boost Software License, Version 1.0. (See accompanying file 
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "test.hpp"
#include "check_type.hpp"
#ifdef TEST_STD
#  include <type_traits>
#else
#  include <boost/type_traits/add_reference.hpp>
#endif

BOOST_DECL_TRANSFORM_TEST(add_reference_test_1, ::boost::add_reference, const, const&)
BOOST_DECL_TRANSFORM_TEST(add_reference_test_2, ::boost::add_reference, volatile, volatile&)
BOOST_DECL_TRANSFORM_TEST(add_reference_test_3, ::boost::add_reference, *, *&)
BOOST_DECL_TRANSFORM_TEST2(add_reference_test_4, ::boost::add_reference, &)
BOOST_DECL_TRANSFORM_TEST(add_reference_test_5, ::boost::add_reference, const &, const&)
BOOST_DECL_TRANSFORM_TEST(add_reference_test_6, ::boost::add_reference, &, &)
BOOST_DECL_TRANSFORM_TEST(add_reference_test_7, ::boost::add_reference, *volatile, *volatile&)
BOOST_DECL_TRANSFORM_TEST(add_reference_test_8, ::boost::add_reference, const [2], const (&)[2])
BOOST_DECL_TRANSFORM_TEST(add_reference_test_9, ::boost::add_reference, const &, const&)
BOOST_DECL_TRANSFORM_TEST(add_reference_test_10, ::boost::add_reference, const*, const*&)
BOOST_DECL_TRANSFORM_TEST(add_reference_test_11, ::boost::add_reference, volatile*, volatile*&)
BOOST_DECL_TRANSFORM_TEST(add_reference_test_12, ::boost::add_reference, const[2][3], const (&)[2][3])
BOOST_DECL_TRANSFORM_TEST(add_reference_test_13, ::boost::add_reference, (&)[2], (&)[2])

TT_TEST_BEGIN(add_reference)

   add_reference_test_1();
   add_reference_test_2();
   add_reference_test_3();
   add_reference_test_4();
   add_reference_test_5();
   add_reference_test_6();
   add_reference_test_7();
   add_reference_test_8();
   add_reference_test_9();
   add_reference_test_10();
   add_reference_test_11();
   add_reference_test_12();
   add_reference_test_13();

TT_TEST_END








