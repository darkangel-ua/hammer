#pragma once
#include <hammer/core/main_target.h>

namespace hammer {

class basic_build_target;

class pch_main_target : public main_target {
   public:
      pch_main_target(const basic_meta_target* mt,
                      const main_target& owner,
                      const std::string& name,
                      const target_type* t,
                      const feature_set* props);

      const basic_build_target& pch_header() const { return *pch_header_; }
      const basic_build_target& pch_source() const { return *pch_source_; }
      const basic_build_target& pch_product() const { return *pch_product_; }
      build_nodes_t generate() const override;

   protected:
      location_t intermediate_dir_impl() const override;

   private:
      const main_target& owner_;
      mutable const basic_build_target* pch_header_;
      mutable const basic_build_target* pch_source_;
      mutable const basic_build_target* pch_product_;
};

}
