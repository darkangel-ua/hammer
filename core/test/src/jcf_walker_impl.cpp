#include "stdafx.h"
#include "jcf_walker_impl.h"
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <hammer/core/project.h>
#include <hammer/core/main_target.h>
#include <iostream>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/target_type.h>
#include <hammer/core/feature.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature_registry.h>
#include <hammer/core/fs_helpers.h>
#include <hammer/core/meta_target.h>

using namespace hammer;
using namespace std;
using boost::format;

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

      BOOST_CHECK_MESSAGE(false, format("Target '%s' was not found") % id);
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

         BOOST_CHECK_MESSAGE(false, format("Target '%s' was not found") % id);
         return 0;
      }
      else
      {
         BOOST_CHECK_MESSAGE(false, format("'%s' is not a main target") % id);
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
   const target_type* et = 0;
   
   et = eng->get_type_registry().find(type_tag(type_id));
   if (et == 0)
   {
      BOOST_CHECK_MESSAGE(false, format("Type '%s'  is unknown.") % type_id);
      return;
   }

   if (!bt->type().equal_or_derived_from(*et))
      BOOST_CHECK_MESSAGE(false, format("Expected type '%s' but got '%s'. Target '%s'")
                                    % type_id
                                    % bt->type().tag().name()
                                    % bt->name());
}

void* get_features(void* t)
{
   if (!t)
      return 0;

   const basic_target* bt = static_cast<const basic_target*>(t);
   return const_cast<feature_set*>(&bt->properties());
}

void check_feature(void* e, void* t, void* features, const pANTLR3_COMMON_TOKEN name_token, const pANTLR3_COMMON_TOKEN value_token)
{
   const char* name = reinterpret_cast<const char*>(name_token->getText(name_token)->chars);
   const char* value = reinterpret_cast<const char*>(name_token->getText(value_token)->chars);
   const feature_set* fs = static_cast<const feature_set*>(features);
   const basic_target* bt = static_cast<const basic_target*>(t);
   engine* eng = static_cast<engine*>(e);
   const feature_def* global_def = eng->feature_registry().find_def(name);
   const feature_def* local_def = bt->get_project().local_feature_registry().find_def(name);
   const feature_def& fd = global_def ? *global_def
                                      : bt->get_project().local_feature_registry().get_def(name);
   
   if (fd.attributes().path)
   {
      feature_set::const_iterator f = fs->find(name);
      if (f == fs->end())
      {
         BOOST_CHECK_MESSAGE(false, format("%s(%d) : Expected feature '%s' with value '%s' was not found")
                                       % reinterpret_cast<const char*>(name_token->input->fileName->chars)
                                       % name_token->line
                                       % name
                                       % value);
         return;
      }

      location_t p1((**f).get_path_data().project_->location() / (**f).value());
      location_t p2(bt->get_main_target()->location());
      p1.normalize();
      p2.normalize();
      location_t p = relative_path(p1, p2);
      p.normalize();
      location_t expected_path(value);
      expected_path.normalize();
      if (p != expected_path)      
         BOOST_CHECK_MESSAGE(false, format("%s(%d) : Expected feature '%s' with value '%s' but found value '%s'")
                                       % reinterpret_cast<const char*>(name_token->input->fileName->chars)
                                       % name_token->line
                                       % name
                                       % value
                                       % p);
      return;
   }

   if (!fs->find(name, value))
   {
      BOOST_CHECK_MESSAGE(false, format("%s(%d) : Expected feature '%s' with value '%s' for target '%s\\\\%s' was not found")
                                    % reinterpret_cast<const char*>(name_token->input->fileName->chars)
                                    % name_token->line
                                    % name
                                    % value
                                    % bt->get_main_target()->location()
                                    % bt->name());
      return;
   }
} 

void check_not_feature(void* e, void* t, void* features, const char* name, const char* value)
{
   const feature_set* fs = static_cast<const feature_set*>(features);
   if (fs->find(name, value))
      BOOST_CHECK_MESSAGE(false, format("Unexpected feature '%s' with value '%s' found in properties") % name % value);
}

void check_location(void* t, const char* location)
{
   const basic_target* bt = static_cast<const basic_target*>(t);
   if (const main_target* mt = dynamic_cast<const main_target*>(bt))
   {
      if (mt->location() != location)
         BOOST_CHECK_MESSAGE(false, format("Expected location '%s' but got '%s'") % location % mt->location());
   }
   else
      BOOST_CHECK_MESSAGE(false, format("Target '%s' is not a main target") % bt->name());
}

void check_number_of_sources(void* t, const char* number)
{
   const basic_target* bt = static_cast<const basic_target*>(t);
   const main_target* mt = dynamic_cast<const main_target*>(bt);
   BOOST_REQUIRE(mt);
   const size_t expected_number_of_sources = boost::lexical_cast<size_t>(number);
   BOOST_CHECK_EQUAL(mt->sources().size(), expected_number_of_sources);
}
