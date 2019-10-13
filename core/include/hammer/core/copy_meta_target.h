#pragma once
#include <hammer/core/meta_target.h>

namespace hammer {

class copy_meta_target : public meta_target {
   public:
      copy_meta_target(hammer::project* p,
                       const std::string& name,
                       const location_t& destination,
                       const std::vector<const target_type*>& types_to_copy,
                       const bool recursive);
   protected:
      main_target*
      construct_main_target(const main_target* owner,
                            const feature_set* properties) const override;
   public:
      const location_t destination_; // full path
      const std::vector<const target_type*> types_to_copy_;
      const bool recursive_;
};

}
