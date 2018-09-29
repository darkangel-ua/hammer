#include "stdafx.h"
#include <hammer/core/basic_target.h>
#include <hammer/core/main_target.h>
#include <hammer/core/meta_target.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <boost/crypto/md5.hpp>

namespace hammer{

const location_t& basic_target::location() const
{
   return main_target_->location();
}

void basic_target::properties(const feature_set* p) 
{ 
   hash_.reset();
   features_ = p; 
}

const std::string& basic_target::hash_string() const
{
   if (!hash_)
      hash_ = hash_string(properties(), *main_target_);
   
   return *hash_;
}

std::string basic_target::hash_string(const feature_set& fs, const main_target& mt)
{
   feature_set::const_iterator i = fs.find("mangling");
   if (i == fs.end() || (**i).value() == "md5")
   {
      std::ostringstream s;
      dump_for_hash(s, fs);
      static_cast<const basic_target&>(mt).additional_hash_string_data(s);
      return boost::crypto::md5(s.str()).to_string();
   }
   else
      return fs.get("variant").value();
}

const basic_meta_target* basic_target::get_meta_target() const
{
   return main_target_->get_meta_target();
}

const project* basic_target::get_project() const
{
   return main_target_->get_meta_target()->get_project();
}

engine& basic_target::get_engine() const
{
   return main_target_->get_meta_target()->get_engine();
}

location_t basic_target::full_path() const
{
   location_t l = location() / name();
   l.normalize();
   return l;
}

build_nodes_t
basic_target::generate() const
{
   if (!generate_cache_)
      generate_cache_ = generate_impl();

   return *generate_cache_;
}

}
