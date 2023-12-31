//  (C) Copyright Gennadiy Rozental 2001-2005.
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at 
//  http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org/libs/test for the library home page.
//
//  File        : $RCSfile: boost_check_equal_str.cpp,v $
//
//  Version     : $Revision: 1.2 $
//
//  Description : basic_cstring unit test
// *****************************************************************************

#include <boost/test/test_tools.hpp>

int test_main( int, char *[] )
{
    char const* buf_ptr_cch     = "abc";
    char const  buf_array_cch[] = "abc";
    char*       buf_ptr_ch      = "abc";
    char        buf_array_ch[]  = "abc";
    std::string buf_str         = "abc";

    BOOST_CHECK_EQUAL(buf_ptr_cch, buf_ptr_cch);
    BOOST_CHECK_EQUAL(buf_ptr_cch, buf_array_cch);
    BOOST_CHECK_EQUAL(buf_ptr_cch, buf_ptr_ch);
    BOOST_CHECK_EQUAL(buf_ptr_cch, buf_array_ch);
    BOOST_CHECK_EQUAL(buf_ptr_cch, buf_str);

    BOOST_CHECK_EQUAL(buf_array_cch, buf_ptr_cch);
    BOOST_CHECK_EQUAL(buf_array_cch, buf_array_cch);
    BOOST_CHECK_EQUAL(buf_array_cch, buf_ptr_ch);
    BOOST_CHECK_EQUAL(buf_array_cch, buf_array_ch);
    BOOST_CHECK_EQUAL(buf_array_cch, buf_str);

    BOOST_CHECK_EQUAL(buf_ptr_ch, buf_ptr_cch);
    BOOST_CHECK_EQUAL(buf_ptr_ch, buf_array_cch);
    BOOST_CHECK_EQUAL(buf_ptr_ch, buf_ptr_ch);
    BOOST_CHECK_EQUAL(buf_ptr_ch, buf_array_ch);
    BOOST_CHECK_EQUAL(buf_ptr_ch, buf_str);

    BOOST_CHECK_EQUAL(buf_array_ch, buf_ptr_cch);
    BOOST_CHECK_EQUAL(buf_array_ch, buf_array_cch);
    BOOST_CHECK_EQUAL(buf_array_ch, buf_ptr_ch);
    BOOST_CHECK_EQUAL(buf_array_ch, buf_array_ch);
    BOOST_CHECK_EQUAL(buf_array_ch, buf_str);

    BOOST_CHECK_EQUAL(buf_str, buf_ptr_cch);
    BOOST_CHECK_EQUAL(buf_str, buf_array_cch);
    BOOST_CHECK_EQUAL(buf_str, buf_ptr_ch);
    BOOST_CHECK_EQUAL(buf_str, buf_array_ch);
    BOOST_CHECK_EQUAL(buf_str, buf_str);

    return 0;
}

// EOF
