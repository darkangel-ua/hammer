#pragma once
#include <string>
#include <vector>
#include <utility>

namespace hammer
{
   class type;
   class feature_set;
   class basic_target;
   class main_target;

   class generator
   {
      public:
         struct consumable_type
         {
            consumable_type(const type* t, unsigned int f, const feature_set* fs) : type_(t), flags_(f), features_(fs) {}   
            
            const type* type_;
            unsigned int flags_;
            const feature_set* features_;
         };

         struct produced_type
         {
            produced_type(const type* t, unsigned int f) : type_(t), flags_(f) {}

            const type* type_;
            unsigned int flags_;
         };

         typedef std::vector<consumable_type> consumable_types;
         typedef std::vector<produced_type> prodused_types;

         generator(const std::string& name,
                   const consumable_types& source_types,
                   const prodused_types& target_types);
      
         const std::string& name() const { return name_; }
         virtual std::vector<basic_target*> transform(main_target* to_target, 
                                                      const std::vector<basic_target*>& from_targets) const;

      private:
         const std::string name_;
         consumable_type source_types_;
         prodused_types target_types_;
   };
}