/* Boost.MultiIndex test for comparison functions.
 *
 * Copyright 2003-2005 Joaqu�n M L�pez Mu�oz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#include "test_comparison.hpp"

#include <boost/config.hpp> /* keep it first to prevent nasty warns in MSVC */
#include "pre_multi_index.hpp"
#include "employee.hpp"
#include <boost/test/test_tools.hpp>

using namespace boost::multi_index;

template<typename Value>
struct lookup_list{
  typedef multi_index_container<
    Value,
    indexed_by<
      sequenced<>,
      ordered_non_unique<identity<Value> >
    >
  > type;
};

void test_comparison()
{
  employee_set              es;
  employee_set_by_age&      i2=get<2>(es);
  employee_set_as_inserted& i3=get<3>(es);
  es.insert(employee(0,"Joe",31,1123));
  es.insert(employee(1,"Robert",27,5601));
  es.insert(employee(2,"John",40,7889));
  es.insert(employee(3,"Albert",20,9012));
  es.insert(employee(4,"John",57,1002));

  employee_set              es2;
  employee_set_by_age&      i22=get<age>(es2);
  employee_set_as_inserted& i32=get<3>(es2);
  es.insert(employee(0,"Joe",31,1123));
  es.insert(employee(1,"Robert",27,5601));
  es.insert(employee(2,"John",40,7889));
  es.insert(employee(3,"Albert",20,9012));

  BOOST_CHECK(es==es&&es<=es&&es>=es&&
              i22==i22&&i22<=i22&&i22>=i22&&
              i32==i32&&i32<=i32&&i32>=i32);
  BOOST_CHECK(es!=es2&&es2<es&&es>es2&&!(es<=es2)&&!(es2>=es));
  BOOST_CHECK(i2!=i22&&i22<i2&&i2>i22&&!(i2<=i22)&&!(i22>=i2));
  BOOST_CHECK(i3!=i32&&i32<i3&&i3>i32&&!(i3<=i32)&&!(i32>=i3));

  lookup_list<int>::type  l1;
  lookup_list<char>::type l2;
  lookup_list<long>::type l3;

  l1.push_back(3);
  l1.push_back(4);
  l1.push_back(5);
  l1.push_back(1);
  l1.push_back(2);

  l2.push_back(char(3));
  l2.push_back(char(4));
  l2.push_back(char(5));
  l2.push_back(char(1));
  l2.push_back(char(2));

  l3.push_back(long(3));
  l3.push_back(long(4));
  l3.push_back(long(5));
  l3.push_back(long(1));

  BOOST_CHECK(l1==l2&&l1<=l2&&l1>=l2);
  BOOST_CHECK(
    get<1>(l1)==get<1>(l2)&&get<1>(l1)<=get<1>(l2)&&get<1>(l1)>=get<1>(l2));
  BOOST_CHECK(l1!=l3&&l3<l1&&l1>l3);
  BOOST_CHECK(
    get<1>(l1)!=get<1>(l3)&&get<1>(l1)<get<1>(l3)&&get<1>(l3)>get<1>(l1));
}
