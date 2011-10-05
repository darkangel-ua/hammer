#if !defined(h_e7c984c0_a4f6_42e2_89b9_ec70352d581c)
#define h_e7c984c0_a4f6_42e2_89b9_ec70352d581c

#include "basic_target.h"
#include "pstring.h"
#include "location.h"
#include <vector>
#include <iosfwd>

namespace hammer
{
   class meta_target;
   class feature_set;
   class mksig_action;

   class main_target : public basic_target
   {
      public:
         typedef std::vector<basic_target*> sources_t;
         // FIXME I think this should be std::vector<main_target*>
         typedef sources_t dependencies_t;

         main_target(const meta_target* mt, 
                     const pstring& name, 
                     const target_type* t, 
                     const feature_set* props,
                     pool& p);

         virtual const location_t& location() const;
         void sources(const sources_t& srcs);
         const sources_t& sources() const { return sources_; }
         void dependencies(const dependencies_t& deps);
         const dependencies_t& dependencies() const { return dependencies_; }
         void src_dependencies(const dependencies_t& deps);
         const dependencies_t& src_dependencies() const { return src_dependencies_; }
         const hammer::meta_target* get_meta_target() const { return meta_target_; }
         virtual build_nodes_t generate();
         const location_t& intermediate_dir() const;
         boost::intrusive_ptr<const hammer::build_node> build_node() const { return build_node_; }
         std::string version() const;
         const std::string& hash() const;

      protected:
         virtual void add_additional_dependencies(hammer::build_node& generated_node) const;
         virtual void additional_hash_string_data(std::ostream& s) const;
         virtual location_t intermediate_dir_impl() const;
         // must be private, but bad design require me to place it here for file_main_target
         void generate_and_add_dependencies(hammer::build_node& node);

      private:
         const hammer::meta_target* meta_target_;
         sources_t sources_;
         sources_t dependencies_;
         sources_t src_dependencies_;
         boost::intrusive_ptr<hammer::build_node> build_node_;
         mutable location_t intermediate_dir_;
         std::vector<boost::intrusive_ptr<hammer::build_node> > generate_cache_;
         bool generate_cache_filled_;
         static boost::shared_ptr<mksig_action> mksig_action_;

         virtual void timestamp_info_impl() const;
         
         build_node_ptr 
         add_intermediate_dir_dependency(hammer::build_node& generated_node) const;
         
         void add_hamfile_dependency(hammer::build_node& node,
                                     const build_node_ptr& intermediate_dir_node) const;
         void add_this_target_dependency(hammer::build_node& node, 
                                         const build_nodes_t& nodes) const;
   };
}

#endif //h_e7c984c0_a4f6_42e2_89b9_ec70352d581c
