#include <iostream>
#include <boost/date_time/gregorian/gregorian.hpp>

int main()
{
   std::cout << boost::gregorian::day_clock::local_day() << std::endl;

   return 1;
}
