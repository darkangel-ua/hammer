#if !defined(h_fd9d533e_506e_440b_b081_a3cf5544771d)
#define h_fd9d533e_506e_440b_b081_a3cf5544771d

#include "generator.h"
#include <boost/ptr_container/ptr_map.hpp>

namespace hammer
{
   class basic_target;
   class main_target;

   class generator_registry
   {
      public:
         typedef boost::ptr_map<std::string, generator> generators_t;

         void insert(std::auto_ptr<generator> g);
         std::vector<boost::intrusive_ptr<build_node> > construct(main_target* mt) const;

      private:
         generators_t generators_;

         std::vector<const generator*> find_viable_generators(const type& t, bool allow_composite) const;
         bool transform_to_consumable(const generator& target_generator, 
                                      const generator& current_generator,
                                      boost::intrusive_ptr<build_node> t, 
                                      std::vector<boost::intrusive_ptr<build_node> >* result,
                                      const feature_set& props,
                                      const main_target& owner) const;
         bool transform(const generator& target_generator, 
                        const generator& current_generator, 
                        const basic_target* t,
                        boost::intrusive_ptr<build_node> target_owner, 
                        std::vector<boost::intrusive_ptr<build_node> >* result,
                        const feature_set& props,
                        const main_target& owner) const;
   };
}

#endif //h_fd9d533e_506e_440b_b081_a3cf5544771d
