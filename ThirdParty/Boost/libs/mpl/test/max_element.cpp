
// Copyright Eric Friedman 2002-2003
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Source: /cvsroot/boost/boost/libs/mpl/test/max_element.cpp,v $
// $Date: 2004/09/02 15:41:35 $
// $Revision: 1.3 $

#include <boost/mpl/max_element.hpp>

#include <boost/mpl/list_c.hpp>
#include <boost/mpl/aux_/test.hpp>

MPL_TEST_CASE()
{
    typedef list_c<int,3,4,2,0,-5,8,-1,7>::type numbers;
    typedef max_element< numbers >::type iter;
    typedef deref<iter>::type max_value;
    
    MPL_ASSERT_RELATION( max_value::value, ==, 8 );
}
