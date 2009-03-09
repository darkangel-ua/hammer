#include "stdafx.h"
#include <hammer/core/c_scanner.h>
#include <hammer/core/scaner_context.h>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/ptr_container/ptr_unordered_map.hpp>
#include <boost/scoped_ptr.hpp>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/basic_meta_target.h>
#include <hammer/core/basic_target.h>
#include <hammer/core/build_environment.h>
#include <hammer/core/hashed_location_serialization.h>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/regex.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/collections_save_imp.hpp>
#include <boost/serialization/collections_load_imp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/filesystem/fstream.hpp>

using namespace std;
using namespace boost::posix_time;
namespace fs = boost::filesystem;

namespace boost
{
   template<typename T>
   bool operator == (const reference_wrapper<T>& lhs, const reference_wrapper<T>& rhs)
   {
      return lhs.get() == rhs.get();
   }
}

namespace hammer{

// struct c_scanner_cache
// {
//    struct item
//    {
//       template<class Archive>
//       void serialize(Archive & ar, const unsigned int version)
//       {
//          ar & timestamp_ & included_files_;
//       }
// 
//       boost::posix_time::ptime timestamp_;
//       c_scanner::included_files_t included_files_;
//    };
// 
//    BOOST_SERIALIZATION_SPLIT_MEMBER()
//    template<class Archive>
//    void save(Archive & ar, const unsigned int version) const
//    {
//       boost::serialization::stl::save_collection<Archive, items_t>(ar, items_);
//    }
// 
//    template<class Archive>
//    void load(Archive & ar, const unsigned int version)
//    {
//       boost::serialization::stl::load_collection<
//          Archive,
//          items_t,
//          boost::serialization::stl::archive_input_map<Archive, items_t>,
//          boost::serialization::stl::no_reserve_imp<items_t> 
//       >(ar, items_);
//    }
// 
//    typedef boost::unordered_map<hashed_location, item> items_t;
//    items_t items_;
// };

struct c_scanner_context : public scanner_context
{
//    struct node;
//    typedef boost::unordered_map<hashed_location, boost::shared_ptr<node> > nodes_t;
//    struct dir_node
//    {
//       location_t dir_;
//       bool exists_;
//       nodes_t nodes_;
//    };
//    typedef boost::unordered_map<location_t, dir_node, boost::hash<location_t>, location_equal_to> dir_nodes_t;
// 
//    typedef std::vector<dir_node*> include_files_dirs_t;
// 
//    struct node
//    {
//       struct variant
//       {
//          variant() : timestamp_(neg_infin) {}
// 
//          const feature_set* build_properties_;
//          ptime timestamp_;
//       };
//       typedef boost::unordered_map<const feature_set*, variant> variants_t;
// 
//       node() : timestamp_(neg_infin) {}
// 
//       hashed_location file_path_;
//       ptime timestamp_;
//       const c_scanner::included_files_t* included_files_;
//       variants_t variants_;
//    };
// 
//    typedef boost::ptr_unordered_map<const feature_set*, include_files_dirs_t> include_files_dirs_cache_t;
//    typedef boost::unordered_map<boost::reference_wrapper<hashed_location const>, ptime> included_files_to_nodes_t;
//    typedef boost::unordered_map<const include_files_dirs_t*, included_files_to_nodes_t> inc_dirs_2_node_cache_t;
   struct dir_node;
   struct file_node;
   typedef boost::unordered_set<hashed_location> locations_t;
   typedef std::vector<const hashed_location*> directories_t;
//   typedef std::vector<const hashed_location*> included_files_t;

   struct included_file
   {
      included_file(const hashed_location* path_part, 
                    const file_node* file,
                    bool quoted) 
         : path_part_(path_part),
           file_(file),
           quoted_(quoted)
      {}

      const hashed_location* path_part_;
      const file_node* file_;
      bool quoted_;
   };
   typedef std::vector<included_file> included_files_t;

   struct file_node
   {
      struct variant
      {
         const directories_t* include_dirs_;
         boost::posix_time::ptime timestamp_;
      };
      
      typedef boost::unordered_map<const directories_t*, variant> variants_t;
      typedef boost::unordered_map<const hashed_location*, boost::posix_time::ptime> timestamps_t;
      
      file_node() : filename_(NULL), included_files_loaded_(false) {}

      const hashed_location* filename_;
      included_files_t included_files_;
      bool included_files_loaded_;
      timestamps_t timestamps_;
      variants_t variants_;
   };
   
   typedef boost::unordered_map<const hashed_location*, file_node> file_nodes_t;
   typedef boost::unordered_set<std::pair<const hashed_location*, const file_node*> > visited_nodes_t;
   typedef boost::unordered_map<const feature_set*, directories_t> features_2_dirs_t;
   typedef boost::unordered_set<const hashed_location*> loaded_dirs_t;
   typedef boost::unordered_map<std::pair<const hashed_location*, 
                                          const hashed_location*>, 
                                const hashed_location*> normalization_cache_t;

   c_scanner_context(const c_scanner& owner,
                     const build_environment& env) 
      : owner_(owner), 
        pattern_("^\\s*#\\s*include\\s*(?:(?:\"([^\"]+)\")|(?:<([^>]+)>))"),
        env_(env)
//        cache_(new c_scanner_cache),
//        cache_is_valid_(false)
   {
//       if (env.cache_directory() != NULL)
//          try_load_cache();
   }
   
   c_scanner_context::~c_scanner_context()
   {
//       if (env_.cache_directory() != NULL && cache_is_valid_ == false)
//          try_save_cache();
   }

   const directories_t& get_include_dirs(const feature_set& properties);
   // location + was insertion flag
   // this function load file nodes when unknown directory encountered
   pair<const hashed_location*, bool> get_cached_location(const hashed_location& v) const;
   const hashed_location& add_and_normalize(const hashed_location& p1, const hashed_location& p2) const;

/*
   pair<ptime, const node*> calculate_timestamp(dir_node& dir, 
                                                const hashed_location& file_path,
                                                const include_files_dirs_t& include_files_dirs,
                                                const feature_set& properties);
   ptime calculate_timestamp(dir_node& origin_dir,
                             const hashed_location& origin_file_path,
                             const c_scanner::included_files_t& included_files,
                             const include_files_dirs_t& include_files_dirs,
                             const feature_set& properties);
   dir_node& get_dir_node(const location_t& path);
*/
   void try_load_cache();
   void try_save_cache();
   void load_directory(const hashed_location& dir);
   const included_files_t extract_includes(const location_t& file, 
                                           const ptime& file_timestamp) const;
   included_file make_included_file(const location_t& include_path, bool quoted) const;

   ptime calculate_timestamp(const hashed_location& origin_dir, 
                             const hashed_location& relative_file_path, 
                             const directories_t& include_dirs,
                             visited_nodes_t& visited_nodes);
   ptime calculate_timestamp_for_known_file(const hashed_location& origin_dir, 
                                            const file_node& file, 
                                            const directories_t& include_dirs,
                                            visited_nodes_t& visited_nodes);

/*
   include_files_dirs_cache_t include_files_dirs_cache_;
   inc_dirs_2_node_cache_t inc_dirs_2_node_cache_;
   dir_nodes_t nodes_;
*/
   const c_scanner& owner_;
   boost::regex pattern_;
   const build_environment& env_;
//   mutable boost::scoped_ptr<c_scanner_cache> cache_;
//   mutable bool cache_is_valid_;
   mutable included_files_t empty_included_files_;
   mutable features_2_dirs_t features_2_dirs_;
   mutable locations_t locations_;
   mutable loaded_dirs_t loaded_dirs_;
   mutable file_nodes_t file_nodes_;
   mutable normalization_cache_t normalization_cache_;
};

void c_scanner_context::try_load_cache()
{
//    try
//    {
//       location_t cache_file_path(*env_.cache_directory() / "c_scanner.cache");
//       if (exists(cache_file_path))
//       {
//          fs::ifstream f(cache_file_path, std::ios_base::binary);
//          if (!f)
//             return;
// 
//          boost::archive::binary_iarchive ar(f);
//          ar >> *cache_;
//          cache_is_valid_ = true;
//       }
//    }
//    catch(...)
//    {
//       cache_.reset(new c_scanner_cache);
//    }
}

void c_scanner_context::try_save_cache()
{
   try
   {
//       fs::create_directories(*env_.cache_directory());
//       fs::ofstream f(*env_.cache_directory() / "c_scanner.cache", std::ios_base::trunc | std::ios_base::binary);
//       boost::archive::binary_oarchive ar(f);
//       ar << *cache_;
   }
   catch(...) {}
}

/*
c_scanner_context::dir_node& c_scanner_context::get_dir_node(const location_t& path)
{
   dir_nodes_t::iterator di = nodes_.find(path);

   if (di != nodes_.end())
      return di->second;
   else
   {
      dir_node node;
      node.dir_ = path;
      node.exists_ = exists(path);
      return nodes_.insert(make_pair(path, node)).first->second;
   }
}
*/

void c_scanner_context::load_directory(const hashed_location& dir)
{
   loaded_dirs_t::const_iterator i = loaded_dirs_.find(&dir);
   if (i != loaded_dirs_.end())
      return;

   if (exists(dir.location()))
   {
      for(fs::directory_iterator i(dir.location()), last = fs::directory_iterator(); i != last; ++i)
      {
         if (!is_directory(i->status()))
         {
            const hashed_location* filename = get_cached_location(i->path().filename()).first;
            
            file_node& file = file_nodes_[filename];
            if (file.filename_ == NULL)
               file.filename_ = filename;

            file.timestamps_.insert(make_pair(&dir, from_time_t(last_write_time(i->path()))));
         }
      }
   }
   
   loaded_dirs_.insert(&dir);
}

static bool find_timestamp(ptime& result, 
                           const c_scanner_context::file_node::timestamps_t& timestamps,
                           const hashed_location& dir)
{
   c_scanner_context::file_node::timestamps_t::const_iterator t = timestamps.find(&dir);
   if (t != timestamps.end())
   {
      result = (std::max)(result, t->second);
      return true;
   }
   else
      return false;
}

ptime c_scanner_context::calculate_timestamp_for_known_file(const hashed_location& file_dir, 
                                                            const file_node& file, 
                                                            const directories_t& include_dirs,
                                                            visited_nodes_t& visited_nodes)
{
   ptime result = neg_infin;
   find_timestamp(result, file.timestamps_, file_dir);

   if (visited_nodes.find(make_pair(&file_dir, &file)) != visited_nodes.end())
      return result;

   visited_nodes.insert(make_pair(&file_dir, &file));

   for(included_files_t::const_iterator i = file.included_files_.begin(), last = file.included_files_.end(); i != last; ++i)
   {
      // Skip files that known to be non existent
      if (i->file_ == NULL)
         continue;

      if (i->quoted_)
      {
         // included in quotes
         const hashed_location& included_file_dir = add_and_normalize(file_dir, *i->path_part_);
         load_directory(included_file_dir);

         if (find_timestamp(result, i->file_->timestamps_, included_file_dir))
         {
            result = (std::max)(result, calculate_timestamp(included_file_dir, 
                                                            *i->file_->filename_, 
                                                            include_dirs,
                                                            visited_nodes));
            continue;
         }
      }

      if (i->file_->timestamps_.empty())
         continue;

      if (i->file_->timestamps_.size() == 1)
      {
         const string& dir = i->file_->timestamps_.begin()->first->location().string();
         const string& included_dir = i->path_part_->location().string();
         if (dir.size() > included_dir.size())
            if (std::equal(included_dir.rbegin(), included_dir.rend(), dir.rbegin()))
            {
               string s = dir.substr(0, dir.size() - included_dir.size());
               // add trailing dot
               if (s.size() > 1 && *s.rbegin() == '/')
                  s += '.';

               if (find(include_dirs.begin(), include_dirs.end(), get_cached_location(s).first) != include_dirs.end())
               {
                  result = (std::max)(result, calculate_timestamp(*i->file_->timestamps_.begin()->first, 
                                                                  *i->file_->filename_, 
                                                                  include_dirs,
                                                                  visited_nodes));
                  continue;
               }
            }
      }

      for(directories_t::const_iterator d = include_dirs.begin(), d_last = include_dirs.end(); d != d_last; ++d)
      {
         const hashed_location& included_file_dir = add_and_normalize(**d, *i->path_part_);
         load_directory(included_file_dir);

         if (find_timestamp(result, i->file_->timestamps_, included_file_dir))
         {
            result = (std::max)(result, calculate_timestamp(included_file_dir, 
                                                            *i->file_->filename_, 
                                                            include_dirs,
                                                            visited_nodes));
            break;
         }
      }
   }

   visited_nodes.erase(visited_nodes.find(make_pair(&file_dir, &file)));

   return result;
}

ptime c_scanner_context::calculate_timestamp(const hashed_location& file_dir, 
                                             const hashed_location& filename, 
                                             const directories_t& include_dirs,
                                             visited_nodes_t& visited_nodes)
{
   file_nodes_t::iterator f = file_nodes_.find(&filename);
   if (f != file_nodes_.end())
   {
      file_node& file = f->second;
      if (!file.included_files_loaded_)
      {
         ptime timestamp = neg_infin;
         file.included_files_ = extract_includes(file_dir.location() / filename.location(), 
                                                 timestamp);
         file.included_files_loaded_ = true;
      }

      file_node::variants_t::const_iterator v = file.variants_.find(&include_dirs);
      if (v != file.variants_.end())
         return v->second.timestamp_;
      else
      {
         file_node::variant new_variant;
         new_variant.include_dirs_ = &include_dirs;
         new_variant.timestamp_ = calculate_timestamp_for_known_file(file_dir, file, include_dirs, visited_nodes);
         file.variants_.insert(make_pair(&include_dirs, new_variant));
         return new_variant.timestamp_;
      }
   }
   else
      return neg_infin;
}

pair<const hashed_location*, bool> c_scanner_context::get_cached_location(const hashed_location& v) const
{
   locations_t::const_iterator i = locations_.find(v);
   if (i != locations_.end())
      return make_pair(&*i, false);
   else
      return make_pair(&*locations_.insert(v).first, true);
}

const hashed_location& c_scanner_context::add_and_normalize(const hashed_location& p1, const hashed_location& p2) const
{
   normalization_cache_t::const_iterator i = normalization_cache_.find(make_pair(&p1, &p2));
   if (i != normalization_cache_.end())
      return *i->second;
   else
   {
      location_t l = p1.location() / p2.location();
      l.normalize();
      const hashed_location* hl = get_cached_location(l).first;
      normalization_cache_.insert(make_pair(make_pair(&p1, &p2), hl));
      return *hl;
   }
}

static location_t get_parent(const location_t& l)
{
   location_t parent = l.parent_path();
   if (parent.empty())
      return parent;
   else
      return parent / ".";
}

const c_scanner_context::directories_t& 
c_scanner_context::get_include_dirs(const feature_set& properties)
{
   features_2_dirs_t::const_iterator i = features_2_dirs_.find(&properties);
   if (i != features_2_dirs_.end())
      return i->second;
   else
   {
      directories_t result;
      for(feature_set::const_iterator i = properties.find("include"), last = properties.end(); i != last; i = properties.find(++i, "include"))
      {
         location_t l = (**i).get_path_data().target_->location() / (**i).value().to_string() / ".";
         l.normalize();
         pair<const hashed_location*, bool> r = get_cached_location(l);
         load_directory(*r.first);
         
         result.push_back(r.first);
      }

      return features_2_dirs_.insert(make_pair(&properties, result)).first->second;
   }
}

/*
pair<ptime, const c_scanner_context::node*>
c_scanner_context::calculate_timestamp(dir_node& dir, 
                                       const hashed_location& file_path,
                                       const include_files_dirs_t& include_files_dirs,
                                       const feature_set& properties)
{
   if (!dir.exists_)
      return make_pair(ptime(neg_infin), static_cast<const c_scanner_context::node*>(NULL));

   nodes_t::iterator i = dir.nodes_.find(file_path);
   if (i != dir.nodes_.end())
   {
      if (i->second->timestamp_ == neg_infin)
         return make_pair(i->second->timestamp_, i->second.get()); //file doesn't exists

      node::variants_t::iterator v_i = i->second->variants_.find(&properties);
      if (v_i != i->second->variants_.end())
         return make_pair(v_i->second.timestamp_, i->second.get());
      else
      {
         node::variant v;
         v.build_properties_ = &properties;
         i->second->variants_.insert(make_pair(&properties, v));
         ptime included_files_timestamp = calculate_timestamp(dir,
                                                              file_path,
                                                              *i->second->included_files_, 
                                                              include_files_dirs, 
                                                              properties);
         node::variant& v_ref = i->second->variants_.find(&properties)->second;
         v_ref.timestamp_ = (std::max)(v_ref.timestamp_, included_files_timestamp);
         return make_pair(v_ref.timestamp_, i->second.get());
      }
   }
   else
   {
      boost::shared_ptr<node> new_node(new node);
      new_node->file_path_ = file_path;
      location_t full_file_path = dir.dir_ / file_path.location();
      bool is_file_exists = exists(full_file_path);
      if (is_file_exists)
      {
         node::variant v;
         v.timestamp_ = boost::posix_time::from_time_t(last_write_time(full_file_path));
         new_node->included_files_ = &owner_.extract_includes(full_file_path, v.timestamp_, *this);
         new_node->timestamp_ = v.timestamp_;
         new_node->variants_.insert(make_pair(&properties, v));
         v.build_properties_ = &properties;
      }
      else
         new_node->included_files_ = &empty_included_files_;

      dir.nodes_.insert(make_pair(file_path, new_node));
      if (is_file_exists)
      {
         ptime included_files_timestamp = calculate_timestamp(dir,
                                                              file_path,
                                                              *new_node->included_files_, 
                                                              include_files_dirs, 
                                                              properties);
         node::variant& v_ref = new_node->variants_.find(&properties)->second;
         v_ref.timestamp_ = (std::max)(v_ref.timestamp_, included_files_timestamp);
         return make_pair(v_ref.timestamp_, new_node.get());
      }

      return make_pair(new_node->timestamp_, new_node.get());
   }
}

ptime c_scanner_context::calculate_timestamp(dir_node& origin_dir,
                                             const hashed_location& origin_file_path,
                                             const c_scanner::included_files_t& included_files,
                                             const include_files_dirs_t& include_files_dirs,
                                             const feature_set& properties)
{
   ptime result = neg_infin;
   dir_node* origin_file_dir = NULL;

   for(c_scanner::included_files_t::const_iterator i = included_files.begin(), last = included_files.end(); i != last; ++i)
   {
      if (!i->second)
      {
         if (origin_file_dir == NULL)
         {
            location_t l = origin_dir.dir_ / origin_file_path.location();
            l.normalize();
            origin_file_dir = &get_dir_node(l.parent_path());
         }
         
         ptime timestamp = calculate_timestamp(*origin_file_dir, i->first, include_files_dirs, properties).first;
         result = (std::max)(result, timestamp);
         if (timestamp != neg_infin)
            continue;
      }
      
      inc_dirs_2_node_cache_t::iterator inc_dir_cache_i = inc_dirs_2_node_cache_.find(&include_files_dirs);
      if (inc_dir_cache_i == inc_dirs_2_node_cache_.end())
         inc_dir_cache_i = inc_dirs_2_node_cache_.insert(make_pair(&include_files_dirs, included_files_to_nodes_t())).first;

      included_files_to_nodes_t::const_iterator if_2_nodes_i = inc_dir_cache_i->second.find(boost::cref(i->first));
      if (if_2_nodes_i != inc_dir_cache_i->second.end())
      {
         result = (std::max)(result, if_2_nodes_i->second);         
         continue;
      }

      for(include_files_dirs_t::const_iterator j = include_files_dirs.begin(), j_last = include_files_dirs.end(); j != j_last; ++j)
      {
         std::pair<ptime, const node*> timestamp_and_node = calculate_timestamp(**j, i->first, include_files_dirs, properties);
         result = (std::max)(result, timestamp_and_node.first);
         if (timestamp_and_node.first != neg_infin)
         {
            inc_dir_cache_i->second.insert(make_pair(boost::cref(i->first), timestamp_and_node.first));
            break;
         }
      }
   }
   
   return result;
}
*/

boost::posix_time::ptime c_scanner::process(const basic_target& t, 
                                            scanner_context& context_outer) const
{
   c_scanner_context& context = static_cast<c_scanner_context&>(context_outer);

   location_t target_path = t.location() / t.name().to_string();
   target_path.normalize();

   const c_scanner_context::directories_t& include_files_dirs = context.get_include_dirs(t.properties());
   const hashed_location& file_dir = *context.get_cached_location(get_parent(target_path)).first;
   context.load_directory(file_dir);
   c_scanner_context::visited_nodes_t visited_nodes;
   ptime result = context.calculate_timestamp(file_dir, 
                                              *context.get_cached_location(target_path.filename()).first, 
                                              include_files_dirs,
                                              visited_nodes);

   // Since our scanner is not perfect we just not report 'something not founded' errors
   if (result == neg_infin)
      return ptime(boost::gregorian::date(1900, 01, 01));
   else
      return result;
}

boost::shared_ptr<scanner_context> c_scanner::create_context(const build_environment& env) const
{
   boost::shared_ptr<scanner_context> result = context_.lock();   
   if (!result)
   {
      result.reset(new c_scanner_context(*this, env));
      context_ = result;
   }

   return result;
}

c_scanner_context::included_file 
c_scanner_context::make_included_file(const location_t& include_path, bool quoted) const
{
   const hashed_location* path_part = get_cached_location(get_parent(include_path)).first;
   const hashed_location* filename = get_cached_location(include_path.filename()).first;
   file_node& file = file_nodes_[filename];
   if (file.filename_ == NULL)
      file.filename_ = filename;

   return included_file(path_part, 
                        &file,
                        quoted);
}

const c_scanner_context::included_files_t 
c_scanner_context::extract_includes(const location_t& file, 
                                    const ptime& file_timestamp) const
{
   boost::iostreams::mapped_file_source in(file.native_file_string());
   // FIXME: May be we should complain about this?
   if (in)
   {
      included_files_t includes;

      for(boost::cregex_iterator i(in.data(), in.data() + in.size(), pattern_), last = boost::cregex_iterator(); i != last; ++i)
      {
         if ((*i)[1].matched)
            includes.push_back(make_included_file(location_t((*i)[1]), true));
         else
            includes.push_back(make_included_file(location_t((*i)[2]), false));
      }
      
      return includes;
   }
   else
      return empty_included_files_;
}

}

