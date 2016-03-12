#ifndef a3e452468_5f2b_4af7_90f0_c011c5e821e6
#define a3e452468_5f2b_4af7_90f0_c011c5e821e6

#include <hammer/core/basic_target.h>

namespace hammer {

class warehouse_target : public basic_target
{
   public:
      warehouse_target(const main_target& mt,
                       const pstring& name,
                       const feature_set& build_request);
      ~warehouse_target();

      virtual build_nodes_t generate() const;

   protected:
      virtual void timestamp_info_impl() const;
};

// throw from warehouse_target::generate to signal that build tree has some libs to download/build
class warehouse_unresolved_target_exception : public std::runtime_error
{
   public:
      warehouse_unresolved_target_exception();
      ~warehouse_unresolved_target_exception() throw();
};

}

#endif
