
// Copyright Aleksey Gurtovoy 2003-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Source: /cvsroot/boost/boost/libs/mpl/test/aux_/preprocessor/token_equal.cpp,v $
// $Date: 2004/09/02 15:41:36 $
// $Revision: 1.3 $

#include <boost/mpl/aux_/preprocessor/token_equal.hpp>

#include <boost/preprocessor/logical/not.hpp>
#include <boost/preprocessor/debug/assert.hpp>

#define AUX_ASSERT_EQUAL(x, y) \
    BOOST_PP_ASSERT( BOOST_MPL_PP_TOKEN_EQUAL(x, y) ) \
/**/

#define AUX_ASSERT_NOT_EQUAL(x, y) \
    BOOST_PP_ASSERT( BOOST_PP_NOT( BOOST_MPL_PP_TOKEN_EQUAL(x, y) ) ) \
/**/

#define BOOST_MPL_PP_TOKEN_EQUAL_apple(x) x
#define BOOST_MPL_PP_TOKEN_EQUAL_orange(x) x

AUX_ASSERT_NOT_EQUAL( apple, abc )
AUX_ASSERT_NOT_EQUAL( abc, apple )
AUX_ASSERT_NOT_EQUAL( apple, orange )
AUX_ASSERT_NOT_EQUAL( orange, apple )

AUX_ASSERT_EQUAL( apple, apple )
AUX_ASSERT_EQUAL( orange, orange )
