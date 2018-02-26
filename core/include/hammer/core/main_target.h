#if !defined(h_e7c984c0_a4f6_42e2_89b9_ec70352d581c)
#define h_e7c984c0_a4f6_42e2_89b9_ec70352d581c

#include "basic_target.h"
#include "location.h"
#include <vector>
#include <iosfwd>

namespace hammer
{
   class basic_meta_target;
   class feature_set;
   class mksig_action;

   class main_target : public basic_target
   {
      public:
         typedef std::vector<basic_target*> sources_t;
         // FIXME I think this should be std::vector<main_target*>
         typedef sources_t dependencies_t;

         main_target(const basic_meta_target* mt,
                     const std::string& name,
                     const target_type* t, 
                     const feature_set* props);

         const location_t& location() const override;
         void sources(const sources_t& srcs);
         const sources_t& sources() const { return sources_; }
         void dependencies(const dependencies_t& deps);
         const dependencies_t& dependencies() const { return dependencies_; }
         const basic_meta_target* get_meta_target() const { return meta_target_; }
         build_nodes_t generate() const override;
         const location_t& intermediate_dir() const;
         boost::intrusive_ptr<const hammer::build_node> build_node() const { return build_node_; }
         std::string version() const;
         const std::string& hash() const;

      protected:
         void additional_hash_string_data(std::ostream& s) const override;
         virtual location_t intermediate_dir_impl() const;
			// some targets, copy for example, don't need signature files
			virtual bool need_signature() const { return true; }

      private:
         const basic_meta_target* meta_target_;
         sources_t sources_;
         sources_t dependencies_;
         // FIXME: should call generate if no build_node_ assigned
         mutable boost::intrusive_ptr<hammer::build_node> build_node_;
         mutable location_t intermediate_dir_;
         mutable std::vector<boost::intrusive_ptr<hammer::build_node> > generate_cache_;
         mutable bool generate_cache_filled_;

         build_nodes_t
			create_intermediate_dirs_build_nodes(const build_nodes_t& build) const;

         void add_this_target_dependency(hammer::build_node& node,
                                         const build_nodes_t& dependencies) const;
			void add_this_target_dependency(build_nodes_t& nodes,
                                         const build_nodes_t& dependencies) const;
			void add_additional_dependencies(build_nodes_t& generated_nodes) const;
			void generate_and_add_dependencies(build_nodes_t& nodes) const;
   };
}

#endif //h_e7c984c0_a4f6_42e2_89b9_ec70352d581c
