#include <hammer/ast/path.h>
#include <hammer/ast/visitor.h>
#include <hammer/ast/casts.h>
#include <hammer/ast/list_of.h>
#include <sstream>

namespace hammer{ namespace ast{

bool path::accept(visitor& v) const
{
   return v.visit(*this);
}

std::string path::to_string() const
{
   std::stringstream s;
   if (root_.valid())
      s << '/';

   bool first = true;
   for (auto e : elements_) {
      if (first)
         first = false;
      else
         s << '/';

      if (is_a<id_expr>(e))
         s << as<id_expr>(e)->id().to_string();
      else {
         assert(as<list_of>(e));
         for (auto we : as<list_of>(e)->values())
            s << as<id_expr>(we)->id().to_string();
      }
   }

   return s.str();
}

template<>
const path* as<path>(const node* v) { return dynamic_cast<const path*>(v); }

template<>
bool is_a<path>(const node& v) { return dynamic_cast<const path*>(&v); }

}}
