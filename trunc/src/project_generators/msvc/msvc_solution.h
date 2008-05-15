#pragma once
#include "../../build_node.h"

namespace hammer
{
   class basic_target;
   class engine;

   namespace project_generators
   {
      class msvc_solution
      {
         public:
            struct impl_t;

            msvc_solution(engine& e);
            void add_target(boost::intrusive_ptr<const build_node> node);
            void write() const;
            ~msvc_solution();

         private:
            impl_t* impl_;
      };
   }
}