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
#include <hammer/src/fs_helpers.h>
#include "jcf_walker_context.h"

using namespace hammer;
using namespace std;

void get_target(void* ctx_, const char* id, int is_top)
{
   jcf_walker_context& ctx = *static_cast<jcf_walker_context*>(ctx_);   
   if (ctx.current_target_ == 0)
      return;

   if (is_top)
   {
      const vector<basic_target*>* targets = static_cast<const vector<basic_target*>*>(ctx.targets_);
      for(vector<basic_target*>::const_iterator i = targets->begin(), last = targets->end(); i != last; ++i)
      {
         if ((**i).name() == id)
         {
            ctx.current_target_ = *i;
            return;
         }
      }

      cout << "checker(0): error: Target '" << id << "' is not found.\n";
      return;
   }
   else
   {
      const basic_target* bt = static_cast<const basic_target*>(ctx.current_target_);
      const main_target* mt = dynamic_cast<const main_target*>(bt);
      if (mt)
      {
         for(vector<basic_target*>::const_iterator i = mt->sources().begin(), last = mt->sources().end(); i != last; ++i)
         {
            if ((**i).name() == id)
            {
               ctx.current_target_ = *i;
               return;
            }
         }

         cout << "checker(0): error: Target '" << id << "' is not found.\n";
         return;
      }
      else
      {
         cout << "checker(0): error: '" << id << "' is not a main target\n";
         return;
      }
   }
}

void check_type(void* ctx_, void* e, const char* type_id)
{
   jcf_walker_context& ctx = *static_cast<jcf_walker_context*>(ctx_);   
   if (!ctx.current_target_)
      return;

   const basic_target* bt = static_cast<const basic_target*>(ctx.current_target_);
   engine* eng = static_cast<engine*>(e);
   const type* et = 0;
   
   et = &eng->get_type_registry().resolve_from_name(type_id);
   if (et == 0)
   {
      cout << "checker(0): error : Type '" << type_id << "' is unknown.\n";
      return;
   }

   if (bt->type() != *et)
      cout << "checker(0): error: Expected type '" << type_id << "' but got '" << bt->type().name() << "'.\n";
}

/*
void get_features(void* ctx_)
{
   jcf_walker_context ctx = static_cast<jcf_walker_context*>(ctx_);   
   if (!ctx.)
      return 0;

   const basic_target* bt = static_cast<const basic_target*>(t);
   return const_cast<feature_set*>(&bt->properties());
}
*/

void check_feature(void* ctx_, const char* name, const char* value)
{
   jcf_walker_context& ctx = *static_cast<jcf_walker_context*>(ctx_);   
   const feature_set& fs = static_cast<const feature_set&>(ctx.current_target_->properties());
   if (!fs.find(name, value))
      cout << "checker(0): error: Expected feature '" << name << "' with value '" << value << "' not found.\n";
} 

void check_location(void* ctx_, const char* location)
{
   jcf_walker_context& ctx = *static_cast<jcf_walker_context*>(ctx_);   
   if (const main_target* mt = dynamic_cast<const main_target*>(ctx.current_target_))
   {
      if (mt->location() != location)
         cout << "checker(0): error: Expected location '"  << location << "' but got '" << mt->location() << "'.\n";
   }
   else
      cout << "checker(0): error: Target '" << ctx.current_target_->name() << "' is not a main target.\n"; 
}
