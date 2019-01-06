#include <iostream>
#include <version.h>

extern const char* cpp_version;

int main() 
{
   std::cout << "h_version is '" << h_version << "'" << std::endl;
   std::cout << "cpp_version is '" << cpp_version << "'" << std::endl;

   return 0;
}
