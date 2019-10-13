#include <libs/static_lib.h>
#include <libs/shared_lib.h>
#include <iostream>

int main()
{
   std::cout << from_static_lib() << std::endl;
   std::cout << from_shared_lib() << std::endl;

   return 0;
}