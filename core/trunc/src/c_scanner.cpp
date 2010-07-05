#include "stdafx.h"
#include <boost/filesystem/convenience.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/ptr_container/ptr_unordered_map.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#include <boost/regex.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/collections_save_imp.hpp>
#include <boost/serialization/collections_load_imp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/filesystem/fstream.hpp>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/basic_meta_target.h>
#include <hammer/core/basic_target.h>
#include <hammer/core/build_environment.h>
#include <hammer/core/hashed_location_serialization.h>
#include <hammer/core/c_scanner.h>
#include <hammer/core/scaner_context.h>

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
   struct dir_node;
   struct file_node;
   typedef boost::unordered_set<hashed_location> locations_t;
   typedef std::vector<const hashed_location*> directories_t;

   struct included_file
   {
      included_file() {}
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
   typedef boost::unordered_map<std::pair<const hashed_location*, const hashed_location*>, 
                                included_files_t> included_files_cache_t;
   struct file_node
   {
      struct variant
      {
         const directories_t* include_dirs_;
         boost::posix_time::ptime timestamp_;
      };

      typedef boost::unordered_map<const directories_t*, variant> variants_t;
      struct file_info
      {
         file_info() : included_files_(NULL) {}
         file_info(const ptime& timestamp) 
            : included_files_(NULL), 
              timestamp_(timestamp)
         {}

         const included_files_t* included_files_;
         ptime timestamp_;
         ptime cached_timestamp_;
         variants_t variants_;
      };

      typedef boost::unordered_map<const hashed_location*, file_info> file_infos_t;
      
      file_node() : filename_(NULL) {}

      const hashed_location* filename_;
      file_infos_t file_infos_;
   };

   struct dir_node_t;
   typedef boost::unordered_multimap<const hashed_location*, boost::reference_wrapper<dir_node_t> > dir_nodes_t;

   struct loaded_dirs_record
   {
      const dir_node_t* where_was_loaded_;
      dir_nodes_t directory_content_;
   };

   typedef boost::unordered_map<const hashed_location*, file_node> file_nodes_t;
   typedef boost::unordered_set<const file_node::file_info*> visited_nodes_t;
   typedef boost::unordered_map<const feature_set*, directories_t> features_2_dirs_t;
   typedef boost::unordered_set<const hashed_location*> non_existen_dirs_t;
   typedef boost::unordered_map<std::pair<const hashed_location*, const dir_node_t*>, loaded_dirs_record> loaded_dirs_t;
   typedef boost::unordered_set<const hashed_location*> suffixes_dirs_t;
   typedef boost::unordered_set<const hashed_location*> bad_suffixes_t;
   typedef boost::unordered_set<const hashed_location*> public_dirs_t;
   typedef boost::unordered_map<std::pair<const hashed_location*, 
                                          const hashed_location*>, 
                                const hashed_location*> normalization_cache_t;
   
   struct suffix_node;
   typedef boost::ptr_unordered_map<const hashed_location* /* path_element */ , suffix_node> suffix_nodes_t;
   typedef boost::ptr_vector<dir_node_t> all_dir_nodes_t;

   struct suffix_node
   {
      const hashed_location* path_element_;
      suffix_nodes_t nodes_;
   };

   struct dir_node_t
   {
      dir_node_t() : loaded_(false) {}

      const hashed_location* full_dir_path_;
      bool loaded_;
      dir_nodes_t nodes_;
   };

   c_scanner_context(const c_scanner& owner,
                     const build_environment& env) 
      : owner_(owner), 
        pattern_("^\\s*#\\s*include\\s*(?:(?:\"([^\"]+)\")|(?:<([^>]+)>))"),
        env_(env),
        cache_is_valid_(false)
   {
       if (env.cache_directory() != NULL)
          try_load_cache();
   }
   
   ~c_scanner_context()
   {
      if (env_.cache_directory() != NULL && cache_is_valid_ == false)
          try_save_cache();
   }

   BOOST_SERIALIZATION_SPLIT_MEMBER()
   template<class Archive>
   void save(Archive & ar, const unsigned int version) const;

   template<class Archive>
   void load(Archive & ar, const unsigned int version);

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

   bool load_directory(const hashed_location& dir, dir_node_t& cur_dir_node, suffix_node* cur_suffix_node) const;
   void load_directory(const hashed_location& dir) const;
   void preload_for_suffix(const hashed_location& path_part) const;
   void load_directories_with_suffix(location_t::const_iterator first, 
                                     location_t::const_iterator last, 
                                     dir_node_t& d_node) const;
   void load_directories_with_suffix(const hashed_location& suffix, bool bad_suffix) const;
   void load_directory_using_suffixes(const hashed_location& dir) const;
   void add_new_suffix_to_tree(location_t::const_iterator first, 
                               location_t::const_iterator last, 
                               suffix_node& s_node) const;
   const included_files_t& extract_includes(const hashed_location& file_dir,
                                            const hashed_location& fileName) const;
   included_file make_included_file(const location_t& include_path, bool quoted) const;

   ptime calculate_timestamp(const hashed_location& origin_dir, 
                             const hashed_location& relative_file_path, 
                             const directories_t& include_dirs,
                             visited_nodes_t& visited_nodes);
   ptime calculate_timestamp_for_known_file(const hashed_location& origin_dir, 
                                            const file_node::file_info& file, 
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
   mutable included_files_t empty_included_files_;
   mutable features_2_dirs_t features_2_dirs_;
   mutable locations_t locations_;
   mutable non_existen_dirs_t non_existen_dirs_;
   mutable loaded_dirs_t loaded_dirs_;
   mutable public_dirs_t public_dirs_;
   mutable suffixes_dirs_t suffixes_dirs_;
   mutable bad_suffixes_t bad_suffixes_;
   mutable file_nodes_t file_nodes_;
   mutable normalization_cache_t normalization_cache_;
   mutable included_files_cache_t included_files_cache_;
   mutable all_dir_nodes_t all_dir_nodes_;
   mutable dir_node_t public_dirs_tree_;
   mutable suffix_node suffixes_tree_;
       
   mutable bool cache_is_valid_;
};

void c_scanner_context::try_load_cache()
{
   try
   {
      location_t cache_file_path(*env_.cache_directory() / "c_scanner.cache");
      if (exists(cache_file_path))
      {
         fs::ifstream f(cache_file_path, std::ios_base::binary);
         if (!f)
            return;

         boost::archive::binary_iarchive ar(f);
         ar >> *this;
         cache_is_valid_ = true;
      }
   }
   catch(...)
   {
      locations_.clear();
      included_files_cache_.clear();
      file_nodes_.clear();
      suffixes_dirs_.clear();
      bad_suffixes_.clear();
      non_existen_dirs_.clear();
      loaded_dirs_.clear();
      public_dirs_.clear();
      public_dirs_tree_.nodes_.clear();
      suffixes_tree_.nodes_.clear();
      all_dir_nodes_.clear();
   }
}

void c_scanner_context::try_save_cache()
{
   try
   {
       fs::create_directories(*env_.cache_directory());
       fs::ofstream f(*env_.cache_directory() / "c_scanner.cache", std::ios_base::trunc | std::ios_base::binary);
       boost::archive::binary_oarchive ar(f);
       ar << *this;
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


void c_scanner_context::load_directory(const hashed_location& dir) const
{
   if (load_directory(dir, public_dirs_tree_, &suffixes_tree_))
   {
      public_dirs_.insert(&dir);
      load_directory_using_suffixes(dir);
   }
}

bool c_scanner_context::load_directory(const hashed_location& dir, dir_node_t& cur_dir_node, suffix_node* cur_suffix_node) const
{
   loaded_dirs_t::const_iterator i = loaded_dirs_.find(make_pair(&dir, &cur_dir_node));
   if (i != loaded_dirs_.end())
   {
      // if directory was loaded but into different cur_node we should copy underlaying nodes to this cur_dir_node
      // and walk over suffixes to load not loaded suffix dirs
      // This could happens for bad suffixes
      if (!cur_dir_node.loaded_ && i->second.where_was_loaded_ != &cur_dir_node)
      {
         cur_dir_node.nodes_ = i->second.directory_content_;

         if (cur_suffix_node != NULL)
         {
            for(dir_nodes_t::const_iterator j = cur_dir_node.nodes_.begin(), j_last = cur_dir_node.nodes_.end(); j != j_last;++i)
            {
               suffix_nodes_t::iterator r = cur_suffix_node->nodes_.find(j->first);
               if (r != cur_suffix_node->nodes_.end())
                  load_directory(*j->second.get().full_dir_path_, j->second, r->second);
            }
         }

         cur_dir_node.loaded_ = true;
         return true;
      }
      else
         return false;
   }

   if (non_existen_dirs_.find(&dir) != non_existen_dirs_.end())
      return false;

   loaded_dirs_record ldr;
   ldr.where_was_loaded_ = &cur_dir_node;

   fs::file_status dir_status = status(dir.location());
   if (exists(dir_status) && is_directory(dir_status))
   {
      for(fs::directory_iterator i(dir.location()), last = fs::directory_iterator(); i != last; ++i)
      {
         fs::file_status st = i->status();
         if (is_directory(st))
         {
            auto_ptr<dir_node_t> new_dir_node(new dir_node_t);
            dir_node_t* new_cur_dir_node = new_dir_node.get();
            const hashed_location* element = get_cached_location(i->path().filename()).first;
            const hashed_location* full_dir_path = get_cached_location(dir.location().parent_path() / i->path().filename() / ".").first;
            new_dir_node->full_dir_path_ = full_dir_path;
            cur_dir_node.nodes_.insert(make_pair(element, boost::ref(*new_dir_node)));
            ldr.directory_content_.insert(make_pair(element, boost::ref(*new_dir_node)));
            all_dir_nodes_.push_back(new_dir_node);
            
            if (cur_suffix_node != NULL)
            {
               suffix_nodes_t::iterator r = cur_suffix_node->nodes_.find(element);
               if (r != cur_suffix_node->nodes_.end())
                  load_directory(*full_dir_path, *new_cur_dir_node, r->second);
            }

            continue;
         }

         if (!is_directory(st))
         {
            const hashed_location* filename = get_cached_location(i->path().filename()).first;
            
            file_node& file = file_nodes_[filename];
            if (file.filename_ == NULL)
               file.filename_ = filename;

            try
            {
               file.file_infos_[&dir].timestamp_ = from_time_t(last_write_time(i->path()));
            }
            catch(...)
            {
               file.file_infos_[&dir].timestamp_ = neg_infin;
               // not all files can be examined because of security reasons, for example
            }
         }
      }

      loaded_dirs_.insert(make_pair(make_pair(&dir, &cur_dir_node), ldr));
      cur_dir_node.loaded_ = true;
      return true;
   }
   else
   {
      non_existen_dirs_.insert(&dir);
      return false;
   }
}

static bool is_bad_suffix(const location_t& suffix)
{
   if (suffix.has_root_name() || suffix.has_root_directory())
      return true;

   if (suffix.empty())
      return false;

   location_t::const_iterator back = --suffix.end();
   for(location_t::const_iterator i = suffix.begin(), last = suffix.end(); i != last; ++i)
      if (*i == ".." || (*i == "." && i != back))
         return true;

   return false;
}

void c_scanner_context::add_new_suffix_to_tree(location_t::const_iterator first, 
                                               location_t::const_iterator last, 
                                               suffix_node& s_node) const
{
   if (first == last)
      return;

   const hashed_location* element = get_cached_location(*first).first;
   suffix_nodes_t::iterator i = s_node.nodes_.find(element);
   if (i == s_node.nodes_.end())
   {
      auto_ptr<suffix_node> new_node(new suffix_node);
      new_node->path_element_ = element;
      i = s_node.nodes_.insert(element, new_node).first;
   }
   
   add_new_suffix_to_tree(++first, last, *i->second);
}

void c_scanner_context::load_directories_with_suffix(location_t::const_iterator first, 
                                                     location_t::const_iterator last, 
                                                     dir_node_t& d_node) const
{
   const hashed_location* cur_element = get_cached_location(*first).first;
   boost::iterator_range<dir_nodes_t::iterator> r = d_node.nodes_.equal_range(cur_element);
   for(dir_nodes_t::iterator i = r.begin(), i_last = r.end(); i != i_last; ++i)
   {
      if (!i->second.get().loaded_)
         load_directory(*i->second.get().full_dir_path_, i->second, NULL);
   }
   
   ++first;
   if (first == last)
      return;

   for(dir_nodes_t::iterator i = r.begin(), i_last = r.end(); i != i_last; ++i)
      load_directories_with_suffix(first, last, i->second);
}

void c_scanner_context::load_directories_with_suffix(const hashed_location& suffix, bool bad_suffix) const
{
   if (bad_suffix)
   {
      typedef vector<const hashed_location*> dirs_t;
      dirs_t dirs(public_dirs_.begin(), public_dirs_.end());
      for(dirs_t::const_iterator i = dirs.begin(), last = dirs.end(); i != last; ++i)
         load_directory(add_and_normalize(**i, suffix), public_dirs_tree_, &suffixes_tree_);
   }
   else
      if (!suffix.location().empty())
      {
         // last dot should be skipped
         add_new_suffix_to_tree(suffix.location().begin(), --suffix.location().end(), suffixes_tree_);
         load_directories_with_suffix(suffix.location().begin(), --suffix.location().end(), public_dirs_tree_);
      }
}

void c_scanner_context::load_directory_using_suffixes(const hashed_location& dir) const
{
   // walk over bad suffixes
   typedef vector<const hashed_location*> suffixes_t;
   suffixes_t suffixes(bad_suffixes_.begin(), bad_suffixes_.end());
   for(suffixes_t::const_iterator i = suffixes.begin(), last = suffixes.end(); i != last; ++i)
      load_directory(add_and_normalize(dir, **i), public_dirs_tree_, &suffixes_tree_);

   // good suffixes already processed by load_directory
}

static bool find_timestamp(ptime& result, 
                           const c_scanner_context::file_node::file_infos_t& file_infos,
                           const hashed_location& dir)
{
   c_scanner_context::file_node::file_infos_t::const_iterator t = file_infos.find(&dir);
   if (t != file_infos.end())
   {
      result = (std::max)(result, t->second.timestamp_);
      return true;
   }
   else
      return false;
}

ptime c_scanner_context::calculate_timestamp_for_known_file(const hashed_location& file_dir, 
                                                            const file_node::file_info& file, 
                                                            const directories_t& include_dirs,
                                                            visited_nodes_t& visited_nodes)
{
   ptime result = file.timestamp_;

   if (visited_nodes.find(&file) != visited_nodes.end())
      return result;

   visited_nodes.insert(&file);

   for(included_files_t::const_iterator i = file.included_files_->begin(), last = file.included_files_->end(); i != last; ++i)
   {
      // Skip files that known to be non existent
      if (i->file_ == NULL)
         continue;

      if (i->quoted_)
      {
         // included in quotes
         const hashed_location& included_file_dir = add_and_normalize(file_dir, *i->path_part_);
         if (find_timestamp(result, i->file_->file_infos_, included_file_dir))
         {
            result = (std::max)(result, calculate_timestamp(included_file_dir, 
                                                            *i->file_->filename_, 
                                                            include_dirs,
                                                            visited_nodes));
            continue;
         }
      }

      if (i->file_->file_infos_.empty())
         continue;

      if (i->file_->file_infos_.size() == 1)
      {
         const string& dir = i->file_->file_infos_.begin()->first->location().string();
         const string& included_dir = i->path_part_->location().string();
         if (dir.size() > included_dir.size())
            if (std::equal(included_dir.rbegin(), included_dir.rend(), dir.rbegin()))
            {
               string s = dir.substr(0, dir.size() - included_dir.size());
               // add trailing dot
               if (s.size() > 1 && *s.rbegin() == '/')
                  s += '.';
               
               const hashed_location* l = get_cached_location(s).first;
               if (find(include_dirs.begin(), include_dirs.end(), l) != include_dirs.end())
               {
                  result = (std::max)(result, calculate_timestamp(*i->file_->file_infos_.begin()->first, 
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
         //load_directory(included_file_dir);

         if (find_timestamp(result, i->file_->file_infos_, included_file_dir))
         {
            result = (std::max)(result, calculate_timestamp(included_file_dir, 
                                                            *i->file_->filename_, 
                                                            include_dirs,
                                                            visited_nodes));
            break;
         }
      }
   }

   visited_nodes.erase(visited_nodes.find(&file));

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
      file_node::file_infos_t::iterator fi = file.file_infos_.find(&file_dir);
      if (fi != file.file_infos_.end())
      {
         //file_node::file_info& fi_ref = fi->second; // unused variable
         if (fi->second.included_files_ == NULL || 
             fi->second.timestamp_ != fi->second.cached_timestamp_)
         {
            fi->second.included_files_ = &extract_includes(file_dir, filename);
            fi->second.cached_timestamp_ = fi->second.timestamp_;
         }

         file_node::variants_t::const_iterator v = fi->second.variants_.find(&include_dirs);
         if (v != fi->second.variants_.end())
            return v->second.timestamp_;
         else
         {
            file_node::variant new_variant;
            new_variant.include_dirs_ = &include_dirs;

            new_variant.timestamp_ = calculate_timestamp_for_known_file(file_dir, fi->second, include_dirs, visited_nodes);
            fi->second.variants_.insert(make_pair(&include_dirs, new_variant));
            return new_variant.timestamp_;
         }
      }
      else
         return neg_infin;
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

void c_scanner_context::preload_for_suffix(const hashed_location& path_part) const
{
   if (is_bad_suffix(path_part.location()))
   {
      if (bad_suffixes_.insert(&path_part).second)
         load_directories_with_suffix(path_part, true);
   }
   else
      if (suffixes_dirs_.insert(&path_part).second)
         load_directories_with_suffix(path_part, false);
}

c_scanner_context::included_file 
c_scanner_context::make_included_file(const location_t& include_path, bool quoted) const
{
   const hashed_location* path_part = get_cached_location(get_parent(include_path)).first;
   preload_for_suffix(*path_part);

   const hashed_location* filename = get_cached_location(include_path.filename()).first;
   file_node& file = file_nodes_[filename];
   if (file.filename_ == NULL)
      file.filename_ = filename;

   return included_file(path_part, 
                        &file,
                        quoted);
}

const c_scanner_context::included_files_t& 
c_scanner_context::extract_includes(const hashed_location& file_dir,
                                    const hashed_location& filename) const
{
   location_t file = file_dir.location() / filename.location();
   boost::iostreams::mapped_file_source in(file.native_file_string());
   // FIXME: May be we should complain about this?
   if (in)
   {
      included_files_t& result = included_files_cache_[make_pair(&file_dir, &filename)];
      result.clear();
      for(boost::cregex_iterator i(in.data(), in.data() + in.size(), pattern_), last = boost::cregex_iterator(); i != last; ++i)
      {
         if ((*i)[1].matched)
            result.push_back(make_included_file(location_t((*i)[1]), true));
         else
            result.push_back(make_included_file(location_t((*i)[2]), false));
      }
      
      cache_is_valid_ = false;
      return result;
   }
   else
      return empty_included_files_;
}

template<class Archive>
void c_scanner_context::save(Archive& ar, const unsigned int version) const
{
   typedef boost::unordered_set<const hashed_location*> reduced_locations_t;   
   reduced_locations_t reduced_locations;

   for(included_files_cache_t::const_iterator i = included_files_cache_.begin(), last = included_files_cache_.end(); i != last; ++i)
   {
      reduced_locations.insert(i->first.first); 
      reduced_locations.insert(i->first.second); 

      for(included_files_t::const_iterator j = i->second.begin(), j_last = i->second.end(); j != j_last; ++j)
      {
         reduced_locations.insert(j->path_part_);
         reduced_locations.insert(j->file_->filename_);
      }
   }

   for(public_dirs_t::const_iterator i = public_dirs_.begin(), last = public_dirs_.end(); i != last; ++i)
      reduced_locations.insert(*i);

   typedef boost::unordered_map<const hashed_location*, unsigned long> location_remapper_t;
   location_remapper_t location_remapper;

   size_t size = reduced_locations.size();
   ar & size;

   unsigned long count = 0;
   for(reduced_locations_t::const_iterator i = reduced_locations.begin(), last = reduced_locations.end(); i != last; ++i, ++count)
   {
      ar & (**i).location().string();
      location_remapper.insert(make_pair(*i, count));
   }

   size = public_dirs_.size();
   ar & size;
   for(public_dirs_t::const_iterator i = public_dirs_.begin(), last = public_dirs_.end(); i != last; ++i)
      ar & location_remapper[*i];

   size = included_files_cache_.size();
   ar & size;

   for(included_files_cache_t::const_iterator i = included_files_cache_.begin(), last = included_files_cache_.end(); i != last; ++i)
   {
      size = i->second.size();

      ar & size 
         & location_remapper[i->first.first] 
         & location_remapper[i->first.second] 
         & file_nodes_[i->first.second].file_infos_[i->first.first].timestamp_;

      for(included_files_t::const_iterator j = i->second.begin(), j_last = i->second.end(); j != j_last; ++j)
         ar & location_remapper[j->path_part_] & location_remapper[j->file_->filename_] & j->quoted_;
   }
}

template<class Archive>
void c_scanner_context::load(Archive & ar, const unsigned int version)
{
   vector<const hashed_location*> locations_index;

   // load locations
   size_t size;
   ar & size;

   locations_index.resize(size);

   string tmp_location;
   for(size_t i = 0; i < size; ++i)
   {
      ar & tmp_location;   
      locations_index[i] = &*locations_.insert(tmp_location).first;
   }

   // load public directories
   ar & size;
   for(size_t i = 0; i < size; ++i)
   {
      unsigned long dir_idx;
      ar & dir_idx;
      load_directory(*locations_index[dir_idx]);
   }

   // load included_files
   ar & size;
   for(size_t i = 0; i < size; ++i)
   {
      size_t j_size;
      unsigned long file_dir_idx, filename_idx;
      ptime cached_timestamp;

      ar & j_size & file_dir_idx & filename_idx & cached_timestamp;

      included_files_t& included_files = included_files_cache_[make_pair(locations_index[file_dir_idx], 
                                                                         locations_index[filename_idx])];
      included_files.resize(j_size);
      for(size_t j = 0; j < j_size; ++j)
      {
         unsigned long include_prefix_idx, file_idx;
         
         ar & include_prefix_idx & file_idx & included_files[j].quoted_;
         
         preload_for_suffix(*locations_index[include_prefix_idx]);

         included_files[j].path_part_ = locations_index[include_prefix_idx];
         file_node& inc_file = file_nodes_[locations_index[file_idx]];
         included_files[j].file_ = &inc_file;
         inc_file.filename_ = locations_index[file_idx];
      }

      file_node& file = file_nodes_[locations_index[filename_idx]];
      file.filename_ = locations_index[filename_idx];
      file_node::file_info& fi = file.file_infos_[locations_index[file_dir_idx]];
      fi.included_files_ = &included_files;
      fi.cached_timestamp_ = cached_timestamp;
      
      assert(fi.cached_timestamp_ != not_a_date_time);
   }
}

}

