#pragma once
#include "generator.h"

namespace hammer
{
   class static_lib_generator : public generator
   {
      public:
         static_lib_generator(hammer::engine& e,
                              const std::string& name,
                              const consumable_types_t& source_types,
                              const producable_types_t& target_types,
                              bool composite,
                              const feature_set* c = 0);
         virtual std::vector<boost::intrusive_ptr<build_node> >
            construct(const type& target_type, 
                      const feature_set& props,
                      const std::vector<boost::intrusive_ptr<build_node> >& sources,
                      const basic_target* t,
                      const pstring* name /* name for composite target*/) const;
      private:
         const type* shared_lib_;
         const type* static_lib_;
         const type* searched_lib_;
   };
}