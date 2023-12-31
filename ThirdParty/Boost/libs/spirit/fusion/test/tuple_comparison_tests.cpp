/*=============================================================================
    Copyright (C) 1999-2003 Jaakko J�rvi
    Copyright (c) 2003 Joel de Guzman

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#include <boost/detail/lightweight_test.hpp>
#include <boost/spirit/fusion/sequence/tuple.hpp>
#include <boost/spirit/fusion/sequence/get.hpp>
#include <boost/spirit/fusion/sequence/equal_to.hpp>
#include <boost/spirit/fusion/sequence/not_equal_to.hpp>
#include <boost/spirit/fusion/sequence/less.hpp>
#include <boost/spirit/fusion/sequence/less_equal.hpp>
#include <boost/spirit/fusion/sequence/greater.hpp>
#include <boost/spirit/fusion/sequence/greater_equal.hpp>

void
equality_test()
{
    using namespace boost::fusion;

    tuple<int, char> t1(5, 'a');
    tuple<int, char> t2(5, 'a');
    BOOST_TEST(t1 == t2);

    tuple<int, char> t3(5, 'b');
    tuple<int, char> t4(2, 'a');
    BOOST_TEST(t1 != t3);
    BOOST_TEST(t1 != t4);
    BOOST_TEST(!(t1 != t2));
}

void
ordering_test()
{
    using namespace boost::fusion;

    tuple<int, float> t1(4, 3.3f);
    tuple<short, float> t2(5, 3.3f);
    tuple<long, double> t3(5, 4.4);
    BOOST_TEST(t1 < t2);
    BOOST_TEST(t1 <= t2);
    BOOST_TEST(t2 > t1);
    BOOST_TEST(t2 >= t1);
    BOOST_TEST(t2 < t3);
    BOOST_TEST(t2 <= t3);
    BOOST_TEST(t3 > t2);
    BOOST_TEST(t3 >= t2);
}

int
main()
{
    equality_test();
    ordering_test();
    return boost::report_errors();
}
