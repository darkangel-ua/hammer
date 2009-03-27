#if !defined(h_24c1034d_1ee4_4e13_8a04_b432735f4fef)
#define h_24c1034d_1ee4_4e13_8a04_b432735f4fef

#include "main_target.h"

namespace hammer
{
   class copy_main_target : public main_target
   {
      public:
         typedef std::vector<const target_type*> types_t;

         copy_main_target(const hammer::meta_target* mt, 
                          const pstring& name, 
                          const target_type* t, 
                          const feature_set* props,
                          pool& p);
         const location_t& destination() const { return destination_; }
         const types_t& types_to_copy() const { return types_to_copy_; }
         bool recursive() const { return recursive_; }
      protected:
         virtual void add_additional_dependencies(hammer::build_node& generated_node) const;

      private:
         location_t destination_;
         types_t types_to_copy_; 
         bool recursive_;
   };
}

#endif //h_24c1034d_1ee4_4e13_8a04_b432735f4fef
