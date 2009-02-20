#include "stdafx.h"
#include <hammer/core/c_scanner.h>
#include <hammer/core/scaner_context.h>
#include <boost/unordered_map.hpp>
#include <boost/ptr_container/ptr_unordered_map.hpp>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/basic_meta_target.h>
#include <hammer/core/basic_target.h>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/regex.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/iostreams/device/mapped_file.hpp>

using namespace std;
using namespace boost::posix_time;

namespace hammer{

struct c_scanner_context : public scanner_context
{
   struct node;
   typedef boost::unordered_map<location_t, boost::shared_ptr<node> > nodes_t;
   typedef std::vector<location_t> include_files_dirs_t;

   struct node
   {
      struct variant
      {
         variant() : timestamp_(neg_infin) {}

         const feature_set* build_properties_;
         ptime timestamp_;
      };
      typedef boost::unordered_map<const feature_set*, variant> variants_t;

      node() : timestamp_(neg_infin) {}

      location_t full_file_path_;
      ptime timestamp_;
      c_scanner::included_files_t included_files_;
      variants_t variants_;
   };

   typedef boost::ptr_unordered_map<const feature_set*, include_files_dirs_t> include_files_dirs_cache_t;
   include_files_dirs_cache_t include_files_dirs_cache_;
   nodes_t nodes_;
   const c_scanner& owner_;
   boost::regex pattern_;

   c_scanner_context(const c_scanner& owner) 
      : owner_(owner), 
        pattern_("^\\s*#\\s*include\\s*(?:(?:\"([^\"]+)\")|(?:<([^>]+)>))") 
   {}

   const include_files_dirs_t& get_include_files_dirs(const feature_set& properties);

   ptime calculate_timestamp(const location_t& full_target_path, 
                             const include_files_dirs_t& include_files_dirs,
                             const feature_set& properties);
   ptime calculate_timestamp(const location_t& origin_file_dir,
                             const c_scanner::included_files_t& included_files,
                             const include_files_dirs_t& include_files_dirs,
                             const feature_set& properties);
};

const c_scanner_context::include_files_dirs_t& 
c_scanner_context::get_include_files_dirs(const feature_set& properties)
{
   include_files_dirs_cache_t::const_iterator i = include_files_dirs_cache_.find(&properties);
   if (i != include_files_dirs_cache_.end())
      return *i->second;
   else
   {
      auto_ptr<include_files_dirs_t> result(new include_files_dirs_t);
      for(feature_set::const_iterator i = properties.find("include"), last = properties.end(); i != last; i = properties.find(++i, "include"))
      {
         location_t l = (**i).get_path_data().target_->location() / (**i).value().to_string();
         l.normalize();
         result->push_back(l);
      }

      return *include_files_dirs_cache_.insert(&properties, result).first->second;
   }
}

ptime c_scanner_context::calculate_timestamp(const location_t& full_target_path, 
                                             const include_files_dirs_t& include_files_dirs,
                                             const feature_set& properties)
{
   nodes_t::iterator i = nodes_.find(full_target_path);
   if (i != nodes_.end())
   {
      node::variants_t::iterator v_i = i->second->variants_.find(&properties);
      if (v_i != i->second->variants_.end())
         return v_i->second.timestamp_;
      else
      {
         node::variant v;
         v.build_properties_ = &properties;
         i->second->variants_.insert(make_pair(&properties, v));
         ptime included_files_timestamp = calculate_timestamp(full_target_path.parent_path(), 
                                                              i->second->included_files_, 
                                                              include_files_dirs, 
                                                              properties);
         node::variant& v_ref = i->second->variants_.find(&properties)->second;
         v_ref.timestamp_ = (std::max)(v_ref.timestamp_, included_files_timestamp);
         return v_ref.timestamp_;
      }
   }
   else
   {
      boost::shared_ptr<node> new_node(new node);
      node::variant v;
      new_node->full_file_path_ = full_target_path;
      if (exists(full_target_path))
      {
         new_node->included_files_ = owner_.extract_includes(full_target_path, *this);
         v.timestamp_ = boost::posix_time::from_time_t(last_write_time(full_target_path));
         new_node->timestamp_ = v.timestamp_;
      }

      v.build_properties_ = &properties;
      new_node->variants_.insert(make_pair(&properties, v));
      nodes_.insert(make_pair(full_target_path, new_node));
      ptime included_files_timestamp = calculate_timestamp(full_target_path.parent_path(), 
                                                           new_node->included_files_, 
                                                           include_files_dirs, 
                                                           properties);
      node::variant& v_ref = new_node->variants_.find(&properties)->second;
      v_ref.timestamp_ = (std::max)(v_ref.timestamp_, included_files_timestamp);
      return v_ref.timestamp_;
   }
}

ptime c_scanner_context::calculate_timestamp(const location_t& origin_file_dir,
                                             const c_scanner::included_files_t& included_files,
                                             const include_files_dirs_t& include_files_dirs,
                                             const feature_set& properties)
{
   ptime result = neg_infin;

   for(c_scanner::included_files_t::const_iterator i = included_files.begin(), last = included_files.end(); i != last; ++i)
   {
      if (!i->second)
      {
         location_t included_file_path(origin_file_dir / i->first);
         included_file_path.normalize();
         ptime timestamp = calculate_timestamp(included_file_path, include_files_dirs, properties);
         result = (std::max)(result, timestamp);
         if (timestamp != neg_infin)
            continue;
      }

      for(include_files_dirs_t::const_iterator j = include_files_dirs.begin(), j_last = include_files_dirs.end(); j != j_last; ++j)
      {
         location_t included_file_path(*j / i->first);
         included_file_path.normalize();

         ptime timestamp = calculate_timestamp(included_file_path, include_files_dirs, properties);
         result = (std::max)(result, timestamp);
         if (timestamp != neg_infin)
            break;
      }
   }
   
   return result;
}

boost::posix_time::ptime c_scanner::process(const basic_target& t, 
                                            scanner_context& context_outer) const
{
   c_scanner_context& context = static_cast<c_scanner_context&>(context_outer);

   location_t target_path = t.location() / t.name().to_string();
   target_path.normalize();

   const c_scanner_context::include_files_dirs_t& include_files_dirs = context.get_include_files_dirs(t.properties());
   return context.calculate_timestamp(target_path, include_files_dirs, t.properties());
}

std::auto_ptr<scanner_context> c_scanner::create_context() const
{
   return std::auto_ptr<scanner_context>(new c_scanner_context(*this));
}

c_scanner::included_files_t c_scanner::extract_includes(const location_t& file, 
                                                        const c_scanner_context& context) const
{
   included_files_t result;
   boost::iostreams::mapped_file_source in(file.native_file_string());
   // FIXME: May be we should complain about this?
   if (in)
   {
      for(boost::cregex_iterator i(in.data(), in.data() + in.size(), context.pattern_), last = boost::cregex_iterator(); i != last; ++i)
      {
         if ((*i)[1].matched)
           result.push_back(make_pair(location_t((*i)[1]), false));
         else
            result.push_back(make_pair(location_t((*i)[2]), true));
      }
   }

   return result;
}

}

