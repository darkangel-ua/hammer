#ifndef HAMMER_CORE_VIRTUAL_TARGET_H
#define HAMMER_CORE_VIRTUAL_TARGET_H

#include <hammer/core/basic_target.h>

namespace hammer {

class virtual_target : public basic_target
{
   public:
      virtual_target(const main_target* mt,
                     const pstring& name,
                     const target_type* t,
                     const feature_set* f);
      build_nodes_t generate() const override;

   protected:
      void timestamp_info_impl() const override;
};

}

#endif // VIRTUAL_TARGET_H
