#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/types.h>
#include <hammer/core/main_target.h>
#include <hammer/core/engine.h>
#include <hammer/core/output_location_strategy.h>
#include <hammer/core/testing_run_meta_target.h>

namespace hammer {

namespace {

class testing_run_main_target : public main_target {
   public:
      testing_run_main_target(const basic_meta_target* mt,
                              const std::string& name,
                              const target_type* t,
                              const feature_set* props)
         : main_target(mt, name, t, props)
      {}

   protected:
      location_t
      intermediate_dir_impl() const override {
         assert(!sources().empty());
         return get_engine().output_location_strategy().compute_output_location(dynamic_cast<const main_target&>(*sources().front()));
      }
};

}

testing_run_meta_target::testing_run_meta_target(hammer::project* p,
                                                 const std::string& name,
                                                 const args& args,
                                                 const bool recheck)
   : typed_meta_target(p, name, {}, {}, p->get_engine().get_type_registry().get(types::TESTING_RUN)),
     args_(args),
     recheck_(recheck)
{
}

testing_run_meta_target::~testing_run_meta_target() = default;

main_target* 
testing_run_meta_target::construct_main_target(const main_target* owner,
                                               const feature_set* properties) const
{
   return new testing_run_main_target{this, name(), &type(), properties};
}

}
