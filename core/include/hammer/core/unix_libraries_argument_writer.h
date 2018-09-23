#pragma once
#include <hammer/core/argument_writer.h>
#include <hammer/core/linker_type.h>

namespace hammer {

class target_type;

class unix_libraries_argument_writer : public argument_writer {
   public:
      unix_libraries_argument_writer(const std::string& name,
                                     linker_type::value linker,
                                     engine& e);
      unix_libraries_argument_writer* clone() const override;
      std::vector<const feature*> valuable_features() const override { return {}; }

   private:
      linker_type::value linker_type_;
      const target_type* shared_lib_type_;
      const target_type* static_lib_type_;
      const target_type* searched_shared_lib_type_;
      const target_type* searched_static_lib_type_;
      const target_type* import_lib_type_;

      void write_impl(std::ostream& output,
                      const build_node& node,
                      const build_environment& environment) const override;
};

}
