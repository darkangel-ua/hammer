#include "stdafx.h"
#include "jcf_walker_impl.h"
#include <hammer/src/project.h>
#include <hammer/src/main_target.h>
#include <iostream>

using namespace hammer;
using namespace std;

void* get_target(const char* id, void* t, int is_top)
{
   if (t == 0)
      return 0;

   if (is_top)
   {
      const vector<basic_target*>* targets = static_cast<const vector<basic_target*>*>(t);
      for(vector<basic_target*>::const_iterator i = targets->begin(), last = targets->end(); i != last; ++i)
      {
         if ((**i).name() == id)
            return *i;
      }

      cout << "checker(0): error: Target '" << id << "' is not found.\n";
      return 0;
   }
   else
   {
      const basic_target* bt = static_cast<const basic_target*>(t);
      const main_target* mt = dynamic_cast<const main_target*>(bt);
      if (mt)
      {
         for(vector<basic_target*>::const_iterator i = mt->sources().begin(), last = mt->sources().end(); i != last; ++i)
         {
            if ((**i).name() == id)
               return *i;
         }

         cout << "checker(0): error: Target '" << id << "' is not found.\n";
         return 0;
      }
      else
      {
         cout << "checker(0): error: '" << id << "' is not a main target\n";
         return 0;
      }
   }
}