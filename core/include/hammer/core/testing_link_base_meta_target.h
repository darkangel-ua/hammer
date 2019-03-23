#pragma once
#include <hammer/core/typed_meta_target.h>

namespace hammer {

class testing_link_base_meta_target : public typed_meta_target {
   public:
      testing_link_base_meta_target(hammer::project* p,
                                    const std::string& name,
                                    const requirements_decl& req,
                                    const target_type& t);
   protected:
      // it depends on instantiation stack so for now we mark it as non-cachable
      bool is_cachable(const main_target*) const override{ return false; }

      sources_decl
      compute_additional_sources(const instantiation_context& ctx,
                                 const main_target& owner) const override;
};

}
