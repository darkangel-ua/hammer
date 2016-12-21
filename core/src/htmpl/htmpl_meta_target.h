#ifndef HAMMER_CORE_HTMPL_HTMPL_META_TARGET
#define HAMMER_CORE_HTMPL_HTMPL_META_TARGET

#include <hammer/core/typed_meta_target.h>

namespace hammer {

class htmpl_meta_target : public typed_meta_target
{
   public:
      htmpl_meta_target(hammer::project* p,
                        const pstring& name,
                        const source_decl& src);
   protected:
      main_target*
      construct_main_target(const main_target* owner,
                            const feature_set* properties) const override;

      void compute_usage_requirements(feature_set& result,
                                      const main_target& constructed_target,
                                      const feature_set& build_request,
                                      const feature_set& computed_usage_requirements,
                                      const main_target* owner) const override;


};

}

#endif
