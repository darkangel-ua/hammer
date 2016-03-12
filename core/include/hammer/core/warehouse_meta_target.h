#ifndef WAREHOUSE_META_TARGET_H
#define WAREHOUSE_META_TARGET_H

#include <hammer/core/basic_meta_target.h>

namespace hammer {

class warehouse_meta_target : public basic_meta_target
{
   public:
      warehouse_meta_target(project& p,
                            const pstring& name);
      ~warehouse_meta_target();

   protected:
      virtual void instantiate_impl(const main_target* owner,
                                    const feature_set& build_request,
                                    std::vector<basic_target*>* result,
                                    feature_set* usage_requirements) const;

};

}

#endif // WAREHOUSE_META_TARGET_H
