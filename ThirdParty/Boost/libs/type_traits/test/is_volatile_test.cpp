
//  (C) Copyright John Maddock 2000. 
//  Use, modification and distribution are subject to the 
//  Boost Software License, Version 1.0. (See accompanying file 
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "test.hpp"
#include "check_integral_constant.hpp"
#ifdef TEST_STD
#  include <type_traits>
#else
#  include <boost/type_traits/is_volatile.hpp>
#endif

TT_TEST_BEGIN(is_volatile)

BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_volatile<void>::value, false);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_volatile<volatile void>::value, true);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_volatile<test_abc1>::value, false);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_volatile<volatile test_abc1>::value, true);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_volatile<int>::value, false);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_volatile<volatile int>::value, true);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_volatile<volatile UDT>::value, true);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_volatile<volatile const UDT>::value, true);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_volatile<incomplete_type>::value, false);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_volatile<foo0_t>::value, false);

TT_TEST_END








