#ifndef a3e452468_5f2b_4af7_90f0_c011c5e821e6
#define a3e452468_5f2b_4af7_90f0_c011c5e821e6

#include <hammer/core/main_target.h>

namespace hammer {

class warehouse_target : public main_target
{
   public:
      warehouse_target(const basic_meta_target& mt,
                       const pstring& name,
                       const feature_set& build_request);
      ~warehouse_target();

      build_nodes_t generate() const override;
};

// throw from warehouse_target::generate to signal that build tree has some libs to download/install
class warehouse_unresolved_target_exception : public std::runtime_error
{
   public:
      warehouse_unresolved_target_exception();
};

}

#endif
