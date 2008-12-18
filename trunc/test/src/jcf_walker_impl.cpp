#include "stdafx.h"
#include "jcf_walker_impl.h"
#include <hammer/src/project.h>
#include <hammer/src/main_target.h>
#include <iostream>
#include <hammer/src/engine.h>
#include <hammer/src/type_registry.h>
#include <hammer/src/type.h>
#include <hammer/src/feature.h>
#include <hammer/src/feature_set.h>
#include <hammer/src/feature_registry.h>
#include <hammer/src/fs_helpers.h>

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

void check_type(void* e, void *t, const char* type_id)
{
   if (!t)
      return;

   const basic_target* bt = static_cast<const basic_target*>(t);
   engine* eng = static_cast<engine*>(e);
   const type* et = 0;
   
   et = eng->get_type_registry().find(type_tag(type_id));
   if (et == 0)
   {
      cout << "checker(0): error : Type '" << type_id << "' is unknown.\n";
      return;
   }

   if (!bt->type().equal_or_derived_from(*et))
      cout << "checker(0): error: Expected type '" << type_id << "' but got '" << bt->type().tag().name() << "'. "
              "Target '" << bt->name() << "'";
}

void* get_features(void* t)
{
   if (!t)
      return 0;

   const basic_target* bt = static_cast<const basic_target*>(t);
   return const_cast<feature_set*>(&bt->properties());
}

void check_feature(void* e, void* t, void* features, const char* name, const char* value)
{
   const feature_set* fs = static_cast<const feature_set*>(features);
   const basic_target* bt = static_cast<const basic_target*>(t);
   engine* eng = static_cast<engine*>(e);
   const feature_def& fd = eng->feature_registry().get_def(name);
   
   if (fd.attributes().path)
   {
      feature_set::const_iterator f = fs->find(name);
      location_t p1((**f).get_path_data().target_->location() / (**f).value().to_string());
      location_t p2(bt->mtarget()->location());
      p1.normalize();
      p2.normalize();
      location_t p = relative_path(p1, p2);
      p.normalize();
      location_t expected_path(value);
      expected_path.normalize();
      if (p != expected_path)      
         cout << "checker(0): error: Expected feature '" << name << "' with value '" << value << "' but found value '" << p << "'.\n";

      return;
   }

   if (!fs->find(name, value))
   {
      cout << "checker(0): error: Expected feature '" << name << "' with value '" << value 
           << "' for target '" << bt->mtarget()->location() << "\\\\" << bt->name() << "' not found.\n";
      return;
   }
} 

void check_not_feature(void* e, void* t, void* features, const char* name, const char* value)
{
   const feature_set* fs = static_cast<const feature_set*>(features);
   if (fs->find(name, value))
      cout << "checker(0): error: Unexpected feature '" << name << "' with value '" << value << " found in properties.\n";
}

void check_location(void* t, const char* location)
{
   const basic_target* bt = static_cast<const basic_target*>(t);
   if (const main_target* mt = dynamic_cast<const main_target*>(bt))
   {
      if (mt->location() != location)
         cout << "checker(0): error: Expected location '"  << location << "' but got '" << mt->location() << "'.\n";
   }
   else
      cout << "checker(0): error: Target '" << bt->name() << "' is not a main target.\n"; 
}
