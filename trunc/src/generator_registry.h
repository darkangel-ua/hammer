#pragma once
#include "generator.h"

namespace hammer
{
   class basic_target;
   class main_target;

   class generator_registry
   {
      public:
         typedef std::map<std::string, generator> generators_t;

         void insert(const generator& g);
         boost::intrusive_ptr<build_node> construct(main_target* mt) const;

      private:
         generators_t generators_;

         std::vector<const generator*> find_viable_generators(const type& t) const;
         bool transform_to_consumable(const generator& target_generator, 
                                      const generator& current_generator,
                                      boost::intrusive_ptr<build_node> t, 
                                      std::vector<boost::intrusive_ptr<build_node> >* result,
                                      const feature_set& props) const;
         bool transform(const generator& target_generator, 
                        const generator& current_generator, 
                        const basic_target* t,
                        boost::intrusive_ptr<build_node> target_owner, 
                        std::vector<boost::intrusive_ptr<build_node> >* result,
                        const feature_set& props) const;
   };
}