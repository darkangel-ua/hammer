#include <string>
#include <iostream>
#include <fstream>

using namespace std;

static const string foo("foo");
static const string some_string("some string");

int main(int argc, char* argv[]) {
   if (argc != 4) {
      cerr << "argc != 4 failed\n";
      return 1;
   }

   if (argv[1] != foo) {
      cerr << "argv[1] != foo failed\n";
      return 2;
   }

   ifstream f(argv[2]);
   if (!f) {
      cerr << "f.open(argv[2]) failed\n";
      return 3;
   }

   if (argv[3] != some_string) {
      cerr << "argv[3] != some_string failed\n";
      return 3;
   }

   return 0;
}
