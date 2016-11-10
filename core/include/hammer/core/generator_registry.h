#if !defined(h_fd9d533e_506e_440b_b081_a3cf5544771d)
#define h_fd9d533e_506e_440b_b081_a3cf5544771d

#include "generator.h"
#include <unordered_map>

namespace hammer
{
   class basic_target;
   class main_target;

   class generator_registry
   {
      public:
         typedef std::unordered_map<std::string, std::unique_ptr<generator>> generators_t;
         typedef std::vector<std::pair<const generator*, const target_type* /* type_to_construct */ > > viable_generators_t;

         void insert(std::unique_ptr<generator> g);
         build_nodes_t construct(const main_target* mt) const;

         viable_generators_t 
         find_viable_generators(const target_type& t, 
                                bool allow_composite,
                                const feature_set& build_properties) const;

      private:
         generators_t generators_;


         bool transform_to_consumable(const generator& target_generator, 
                                      const generator& current_generator,
                                      build_node_ptr t, 
                                      build_nodes_t* result,
                                      const feature_set& props,
                                      const main_target& owner) const;
         bool transform(const generator& target_generator, 
                        const generator& current_generator, 
                        const basic_target* t,
                        build_node_ptr& target_owner, 
                        build_nodes_t* result,
                        const feature_set& props,
                        const main_target& owner) const;
         
         viable_generators_t 
         find_viable_generators(const target_type& t, 
                                bool allow_composite,
                                const feature_set& build_properties,
                                bool full_match) const;
   };
}

#endif //h_fd9d533e_506e_440b_b081_a3cf5544771d
