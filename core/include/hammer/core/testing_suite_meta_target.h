#pragma once
#include <hammer/core/typed_meta_target.h>

namespace hammer {

class testing_suite_meta_target : public typed_meta_target
{
   public:
      testing_suite_meta_target(hammer::project& p,
                                const std::string& name,
                                const sources_decl& sources,
                                const sources_decl& common_sources,
                                const requirements_decl& common_requirements);

      void common_sources(const sources_decl& s);
      const sources_decl&
      common_sources() const { return common_sources_; }

      const requirements_decl&
      common_requirements() const { return common_requirements_; }

   private:
      sources_decl common_sources_;
      const requirements_decl common_requirements_;
};

const testing_suite_meta_target*
find_suite(const project &p,
           const instantiation_context& ctx);

void add_common_requirements(feature_set& build_request,
                             const requirements_decl& req);

}
