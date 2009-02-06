#include "stdafx.h"
#include <hammer/core/source_target.h>
#include <hammer/core/engine.h>

namespace hammer{

   source_target::source_target(const main_target* mt, const pstring& name, 
                                const hammer::type* t, const feature_set* f)
                               : file_target(mt, name, t, f)
   {
   }

   void* source_target::operator new(size_t size, engine* e)
   {
      void* m = e->pstring_pool().malloc(size);
      return m;
   }

   std::vector<boost::intrusive_ptr<build_node> > source_target::generate()
   {
      boost::intrusive_ptr<build_node> result(new build_node);
      result->products_.push_back(this);
      result->targeting_type_ = &this->type();
      return std::vector<boost::intrusive_ptr<build_node> >(1, result);
   }
}
