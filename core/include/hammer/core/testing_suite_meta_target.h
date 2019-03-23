#pragma once
#include <hammer/core/typed_meta_target.h>

namespace hammer {

class testing_suite_meta_target : public typed_meta_target
{
   public:
      testing_suite_meta_target(hammer::project& p,
                                const std::string& name,
                                const sources_decl& sources,
                                const sources_decl& common_sources);

      void common_sources(const sources_decl& s);
      const sources_decl&
      common_sources() const { return common_sources_; }

   private:
      sources_decl common_sources_;
};

}
