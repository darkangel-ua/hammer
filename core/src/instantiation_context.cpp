#include <sstream>
#include <hammer/core/instantiation_context.h>
#include <hammer/core/basic_meta_target.h>

namespace hammer {

void instantiation_context::enter(const basic_meta_target& mt)
{
   auto i = targets_.find(&mt);
   if (i != targets_.end()) {
      std::ostringstream s;

      stack_.push_back(&mt);
      for (auto* mt : stack_)
         s << mt->name() << " at '" << mt->location().string() << "'\n";
      stack_.pop_back();

      throw std::runtime_error("Circular dependency found!\n" + s.str());
   }

   targets_.insert(&mt);
   stack_.push_back(&mt);
}

void instantiation_context::leave()
{
   auto* mt = stack_.back();
   stack_.pop_back();
   targets_.erase(mt);
}

}
