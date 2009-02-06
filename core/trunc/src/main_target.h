#if !defined(h_e7c984c0_a4f6_42e2_89b9_ec70352d581c)
#define h_e7c984c0_a4f6_42e2_89b9_ec70352d581c

#include "basic_target.h"
#include "pstring.h"
#include "location.h"
#include <vector>

namespace hammer
{
   class meta_target;
   class feature_set;

   class main_target : public basic_target
   {
      public:
         typedef std::vector<basic_target*> sources_t;
         // FIXME I think this should be std::vector<main_target*>
         typedef sources_t dependencies_t;

         main_target(const meta_target* mt, 
                     const pstring& name, 
                     const hammer::type* t, 
                     const feature_set* props,
                     pool& p);

         virtual const location_t& location() const;
         void sources(const sources_t& srcs);
         const sources_t& sources() const { return sources_; }
         void dependencies(const dependencies_t& deps);
         const dependencies_t& dependencies() const { return dependencies_; }
         const hammer::meta_target* meta_target() const { return meta_target_; }
         virtual build_nodes_t generate();
         const location_t& intermediate_dir() const;
         boost::intrusive_ptr<const hammer::build_node> build_node() const { return build_node_; }

      protected:
         virtual void add_additional_dependencies(hammer::build_node& generated_node) const;

      private:
         const hammer::meta_target* meta_target_;
         sources_t sources_;
         sources_t dependencies_;
         boost::intrusive_ptr<hammer::build_node> build_node_;
         mutable location_t intermediate_dir_;
         std::vector<boost::intrusive_ptr<hammer::build_node> > generate_cache_;
         bool generate_cache_filled_;

         virtual void timestamp_info_impl() const;
         void generate_and_add_dependencies(hammer::build_node& node);
   };
}

#endif //h_e7c984c0_a4f6_42e2_89b9_ec70352d581c
