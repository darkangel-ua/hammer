#pragma once
#include <boost/variant/variant.hpp>
#include <hammer/core/typed_meta_target.h>

namespace hammer {
   
class testing_intermediate_meta_target : public typed_meta_target {
   public:
      using arg = boost::variant<std::string, boost::filesystem::path>;
      using args = std::vector<arg>;

      testing_intermediate_meta_target(hammer::project* p, 
                                       const std::string& name,
                                       const requirements_decl& req,
                                       const args& args);
      ~testing_intermediate_meta_target();

      const args args_;

   protected:
      sources_decl
      compute_additional_sources(const main_target& owner) const override;
};

}
