/* Simple program that uses the gregorian calendar to find the last
 * day of the month and then display the last day of every month left 
 * in the year.
 */

#include "boost/date_time/gregorian/gregorian.hpp"
#include <iostream>

int
main()
{
  using namespace boost::gregorian;
  
  std::cout << "   Enter Year(ex: 2002): ";
  int year, month;
  std::cin >> year;
  std::cout << "   Enter Month(1..12): ";
  std::cin >> month;
  try {
    int day = gregorian_calendar::end_of_month_day(year,month);
    date end_of_month(year,month,day);

    //Iterate thru by months --
    month_iterator mitr(end_of_month,1);
    date start_of_next_year(year+1, Jan, 1);
    //loop thru the days and print each one
    while (mitr < start_of_next_year){
      std::cout << to_simple_string(*mitr) << std::endl;
      ++mitr;
    }

  }
  catch(...) {
    std::cout << "Invalid Date Entered" << std::endl;
  }
  return 0;

}

/*  Copyright 2001-2004: CrystalClear Software, Inc
 *  http://www.crystalclearsoftware.com
 *
 *  Subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE-1.0 or http://www.boost.org/LICENSE-1.0)
 */

