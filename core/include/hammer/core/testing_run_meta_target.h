#pragma once
#include <boost/variant/variant.hpp>
#include <hammer/core/typed_meta_target.h>

namespace hammer {

class testing_run_meta_target : public typed_meta_target {
   public:
      using arg = boost::variant<std::string, boost::filesystem::path>;
      using args = std::vector<arg>;

      testing_run_meta_target(hammer::project* p,
                              const std::string& name,
                              const args& args,
                              const bool recheck);
      ~testing_run_meta_target() override;

      const args args_;
      const bool recheck_;

   protected:
      main_target*
      construct_main_target(const main_target* owner,
                            const feature_set* properties) const override;
};

}
