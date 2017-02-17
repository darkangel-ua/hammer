#include <iostream>
#include <shared_lib.h>
#include <static_lib.h>

int main() 
{
   std::cout << "From shared lib: " << shared_lib_foo() << std::endl;
   std::cout << "From static lib: " << static_lib_foo() << std::endl;

   return 1;
}

