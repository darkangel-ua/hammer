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

         main_target(const meta_target* mt, 
                     const pstring& name, 
                     const hammer::type* t, 
                     const feature_set* props,
                     pool& p);

         virtual const location_t& location() const;
         void sources(const std::vector<basic_target*>& srcs);
         const sources_t& sources() const { return sources_; }
         const hammer::meta_target* meta_target() const { return meta_target_; }
         virtual std::vector<boost::intrusive_ptr<build_node> > generate();
         const pstring& intermediate_dir() const;
         boost::intrusive_ptr<const hammer::build_node> build_node() const { return build_node_; }

      private:
         const hammer::meta_target* meta_target_;
         sources_t sources_;
         boost::intrusive_ptr<hammer::build_node> build_node_;

         virtual void timestamp_info_impl(timestamp_info_t::getter_policy_t how_to_get) const;
   };
}

#endif //h_e7c984c0_a4f6_42e2_89b9_ec70352d581c
